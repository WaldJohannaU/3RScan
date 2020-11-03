/*******************************************************
* Copyright (c) 2020, Johanna Wald
* All rights reserved.
*
* This file is distributed under the GNU Lesser General Public License v3.0.
* The complete license agreement can be obtained at:
* http://www.gnu.org/licenses/lgpl-3.0.html
********************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <opencv2/opencv.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

class Model {
public:
    // Constructor, expects a filepath to a 3D model.
    Model(const std::string& path, const glm::vec3& rgb_color_filter = glm::vec3(-1, -1, -1)): rgb_color_filter_(rgb_color_filter) {
        use_rgb_color_filter_ = rgb_color_filter_ != glm::vec3(-1, -1, -1);
        this->loadModel(path);
    }
    
    // Draws the model, and thus all its meshes
    void Draw(Shader shader) {
        for (GLuint i = 0; i < this->meshes_.size(); i++) {
            this->meshes_[i].Draw(shader);
        }
    }
private:
    GLint TextureFromFileTest(const char *path, std::string directory) {
        //Generate texture ID and load texture data
        std::string filename{path};
        filename = directory + '/' + filename;
        GLuint textureID;
        glGenTextures(1, &textureID);
        
        cv::Mat image = cv::imread(filename.c_str());
        glDeleteTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());   
        // Assign texture to ID
        glGenerateMipmap(GL_TEXTURE_2D);
        // Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }

    // color filter: only load vertices with this color: when a face contains at least one vertex with this color it is kept, otherwise it is discarded.
    const glm::vec3 rgb_color_filter_;

    // if the rgb_color_filter should be applied when loading the meshes for this model
    bool use_rgb_color_filter_{false};

    //  Model Data
    std::vector<Mesh> meshes_;
    std::string directory_;
    // Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Texture> textures_loaded_;
    
    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(const std::string& path) {
        // Read file via ASSIMP
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // Retrieve the directory path of the filepath
        this->directory_ = path.substr(0, path.find_last_of('/'));
        // Process ASSIMP's root node recursively
        this->processNode(scene->mRootNode, scene);
    }
    
    // Processes a node in a recursive fashion.
    // Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene) {
        // Process each mesh located at the current node
        for (GLuint i = 0; i < node->mNumMeshes; i++) {
            // The node object only contains indices to index the actual objects in the scene.
            // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes_.push_back(this->processMesh(mesh, scene));
        }
        // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (GLuint i = 0; i < node->mNumChildren; i++)
            this->processNode(node->mChildren[i], scene);
    }
    
    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        // Data to fill
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;
        
        std::map<GLuint, bool> filteredIndices; // list of indices of all vertices whose color are not the filter color, if colors should be filtered

        // Walk through each of the mesh's vertices
        for (GLuint i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            // Declare a placeholder vector since assimp uses its own vector class
            // that doesn't directly convert to glm's vec3 class so we transfer the
            // data to this placeholder glm::vec3 first.
            glm::vec3 vector;
            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // Normals
            if (mesh->mNormals) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            if (mesh->mColors[0]) {
                vector.x = mesh->mColors[0][i].r;
                vector.y = mesh->mColors[0][i].g;
                vector.z = mesh->mColors[0][i].b;
                vertex.Color = vector;
                if (use_rgb_color_filter_ && vertex.Color != rgb_color_filter_) {
                    // if vertex does not have the correct color, filter its index.
                    filteredIndices[i] = true;
                }
            }
            // Texture Coordinates
            // Does the mesh contain texture coordinates?
            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex); // we still add filtered vertices since faces reference original indices
        }
        // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.

        // variables for each iteration of the for-loop, instantiated once for reserved memory and higher efficiency
        bool face_contains_non_filtered_index = false; // at least one index is not filtered in this face?
        std::vector<GLuint> indices_of_face; // the indices of the current face

        for (GLuint i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            
            // Retrieve all indices of the face and store them in the indices vector
            for (GLuint j = 0; j < face.mNumIndices; j++) {
                indices_of_face.push_back(face.mIndices[j]);
            }

            // Check if at least one index is not filtered in this face
            for (const auto& index : indices_of_face) {
                if (filteredIndices.find(index) == filteredIndices.end() ){
                    face_contains_non_filtered_index = true;
                    break;
                }
            }

            // Add indices to list if at least one index is not filtered in this face (otherwise ignore the whole face!)
            if (face_contains_non_filtered_index) {
                for (const auto& index : indices_of_face) {
                    indices.push_back(index);
                }
            }

            // reset variables for next iteration
            face_contains_non_filtered_index = false;
            indices_of_face.clear();
        }
        // Process materials
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
            // 1. Diffuse maps
            std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // 2. Specular maps
            std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }
        
        // Return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }
    
    // Checks all material textures of a given type and loads the textures if they're not loaded yet.
    // The required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
        std::vector<Texture> textures;
        for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            GLboolean skip = false;
            for (GLuint j = 0; j < textures_loaded_.size(); j++) {
                if (textures_loaded_[j].path == str) {
                    textures.push_back(textures_loaded_[j]);
                    skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip) {   // If texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFileTest(str.C_Str(), this->directory_);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                // Store it as texture loaded for entire model
                // to ensure we won't unnecesery load duplicate textures.
                this->textures_loaded_.push_back(texture);
            }
        }
        return textures;
    }
};
