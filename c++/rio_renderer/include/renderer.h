/*******************************************************
* Copyright (c) 2020, Johanna Wald
* All rights reserved.
*
* This file is distributed under the GNU Lesser General Public License v3.0.
* The complete license agreement can be obtained at:
* http://www.gnu.org/licenses/lgpl-3.0.html
********************************************************/

#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <Eigen/Dense>

#include "data.h"
#include "intrinsics.h"
#include "model.h"
#include "shader.h"

namespace RIO {

constexpr float kNearPlane{0.1f};
constexpr float kFarPlane{10.0f};

class Renderer {
public:
    Renderer(const std::string& sequence_path, 
             const std::string& data_path, 
             const std::string& scan_id,
             bool save_images = true,
             bool save_depth = true,
             bool save_bounding_boxes = false,
             bool save_occlusion = false,
             float fov_scale = 1.0f,
             bool v2 = true);
    ~Renderer();
    int Init();
    void Render(const bool inc_frame_id, const std::string save_path = "");
    void Render(const int frame_id, const std::string save_path = "");
    void RenderAllFrames(const std::string save_path = "");
    const cv::Mat& GetLabels() const;
    const cv::Mat& GetColor() const;
    const cv::Mat& GetDepth() const;
    const std::map<int, Eigen::Vector4i>& Get2DBoundingBoxes() const;
    const std::map<int, std::string>& GetInstance2Label() const;
    const std::map<int, unsigned long>& GetInstance2Color() const;

    struct VisibilityEntry {
        VisibilityEntry():             
            original_pixel_count(0),
            complete_pixel_count(0),
            ratio(1) { }

        VisibilityEntry(const float original_pixel_count, const float complete_pixel_count):
            original_pixel_count(original_pixel_count),
            complete_pixel_count(complete_pixel_count),
            ratio(original_pixel_count / complete_pixel_count) { }

        float original_pixel_count;
        float complete_pixel_count;
        float ratio; // original / complete
    };
private:
    std::string data_path_{""};
    std::string sequence_path_{""};
    int buffer_width{0};
    int buffer_height{0};
    bool save_images_{true};
    bool save_depth_{true};
    bool save_bounding_boxes_{false};
    bool save_occlusion_{false};
    bool initalized_{false};
    struct RIOData {
        RIOData(const std::string& scan_id): scan_id(scan_id) { }
        const std::string scan_id;
        std::map<int, std::string> instance2label;
        std::map<int, unsigned long> instance2color;
        std::map<unsigned long, int> color2instances;
        std::map<int, Eigen::Vector4i> bboxes;

        // how much of an instance is cut off at the image edges (how much larger is one instance / what is not shown in this frame)
        std::map<int, VisibilityEntry> instances2truncation;

        // how much of the instance is cut off by other objects in the image (the portion that is visible in the frame: how much is occluded by other objects)
        std::map<int, VisibilityEntry> instances2occlusion;

        cv::Mat instances;
        cv::Mat labels;
        cv::Mat color;
        cv::Mat depth;

        cv::Mat labels_fov_scale;
        cv::Mat label_one_instance_only;
    };
    RIOData rio_data_;
    Data data_;
    Data data_fov_scale_;
    Eigen::Matrix4f projection_{Eigen::Matrix4f::Identity()};

    GLFWwindow *window{nullptr};
    Shader* shader_labels_{nullptr};
    Shader* shader_RGB_{nullptr};
    Model* model_RGB_{nullptr};
    Model* model_labels_{nullptr};
    std::map<int, Model*> instance2model_with_instance_only_;
    bool v2_{true};

    void InitGLFW();
    void Render(Model& model, Shader& shader);
    void ReadLabels(cv::Mat& image, cv::Mat& labels);
    void ReadRGB(cv::Mat& image);
    void ReadDepth(cv::Mat& image);
    void CalcTruncations(std::map<int, unsigned long>& instances2color, std::map<int, VisibilityEntry>& instances2truncation, const cv::Mat& labels_fov_scale);
    void CalcOcclusions(std::map<int, unsigned long>& instances2color, std::map<int, VisibilityEntry>& instances2occlusion, const cv::Mat& labels);
    void VerifyTruncationAndOcclusion(std::map<int, VisibilityEntry>& instances2occlusion, std::map<int, VisibilityEntry>& instances2truncation);
    void DrawScene(Model& model, Shader& shader);
    bool LoadObjects(const std::string& obj_file);
};

}; // namespace RIO
