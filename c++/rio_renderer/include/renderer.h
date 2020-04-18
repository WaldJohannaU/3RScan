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
             const std::string& scan_id);
    ~Renderer();
    int Init();
    void Render(const bool inc_frame_id, const std::string save_path = "");
    void Render(const int frame_id, const std::string save_path = "");
    const cv::Mat& GetLabels() const;
    const cv::Mat& GetColor() const;
    const cv::Mat& GetDepth() const;
    const std::map<int, Eigen::Vector4i>& Get2DBoundingBoxes() const;
    const std::map<int, std::string>& GetInstance2Label() const;
    const std::map<int, unsigned long>& GetInstance2Color() const;
private:
    std::string data_path_{""};
    std::string sequence_path_{""};
    int buffer_width{0};
    int buffer_height{0};
    bool initalized_{false};
    struct RIOData {
        RIOData(const std::string& scan_id): scan_id(scan_id) { }
        const std::string scan_id;
        std::map<int, std::string> instance2label;
        std::map<int, unsigned long> instance2color;
        std::map<unsigned long, int> color2instances;
        std::map<int, Eigen::Vector4i> bboxes;

        cv::Mat instances;
        cv::Mat labels;
        cv::Mat color;
        cv::Mat depth;
    };
    RIOData rio_data_;
    Data data_;
    Eigen::Matrix4f projection_{Eigen::Matrix4f::Identity()};

    GLFWwindow *window{nullptr};
    Shader* shader_labels_{nullptr};
    Shader* shader_RGB_{nullptr};
    Model* model_RGB_{nullptr};
    Model* model_labels_{nullptr};

    void InitGLFW();
    void Render(Model& model, Shader& shader);
    void ReadLabels(cv::Mat& image, cv::Mat& labels);
    void ReadRGB(cv::Mat& image);
    void ReadDepth(cv::Mat& image);
    void DrawScene(Model& model, Shader& shader);
    bool LoadObjects(const std::string& obj_file);
};

}; // namespace RIO
