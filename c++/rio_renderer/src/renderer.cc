#include "renderer.h"

#include "json11.hpp"
#include "model.h"
#include "util.h"

namespace RIO {

Renderer::Renderer(const std::string& sequence_path, 
                   const std::string& data_path, 
                   const std::string& scan_id): 
                   data_path_(data_path), sequence_path_(sequence_path),
                   rio_data_(scan_id),  data_(sequence_path) {
}

Renderer::~Renderer() {
    delete shader_labels_;
    delete shader_RGB_;
    delete model_labels_;
    delete model_RGB_;
    glfwTerminate();
}

const std::map<int, Eigen::Vector4i>& Renderer::Get2DBoundingBoxes() const {
    return rio_data_.bboxes;
}

const std::map<int, std::string>& Renderer::GetInstance2Label() const {
    return rio_data_.instance2label;
}

const std::map<int, unsigned long>& Renderer::GetInstance2Color() const {
    return rio_data_.instance2color;
}

const cv::Mat& Renderer::GetLabels() const {
    return rio_data_.labels;
}

const cv::Mat& Renderer::GetColor() const {
    return rio_data_.color;
}

const cv::Mat& Renderer::GetDepth() const {
    return rio_data_.depth;
}

int Renderer::Init() {
    if (!(glfwInit() && data_.LoadIntrinsics()))
        return -1; 

    InitGLFW();
    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(data_.intrinsics.width, data_.intrinsics.height, "Evaluation", nullptr, nullptr);
    if (nullptr == window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
        return EXIT_FAILURE;
    glEnable(GL_DEPTH_TEST);
    // Setup and compile our shaders
    shader_labels_ = new Shader("../res/color3D.vs", "../res/color3D.frag");
    shader_RGB_ = new Shader("../res/textured3D.vs", "../res/textured3D.frag");
    model_labels_ = new Model(data_path_ + "/" + rio_data_.scan_id + "/labels.instances.annotated.ply");
    model_RGB_ = new Model(data_path_ + "/" + rio_data_.scan_id + "/mesh.refined.obj");
    data_.LoadViewMatrix();
    initalized_ = true;
    // We have to read the buffer size because of retrina displays. 
    glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
    return 0;
}

void Renderer::Render(const int frame_id, const std::string save_path) {
    data_.SetFrame(frame_id);
    Render(false, save_path);
}

void Renderer::Render(const bool inc_frame_id, const std::string save_path) {
    if (!initalized_)
        return;
    projection_ = camera_utils::perspective<Eigen::Matrix4f::Scalar>(data_.intrinsics, kNearPlane, kFarPlane);
    GLuint framebuffername = 1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffername);
    DrawScene(*model_RGB_, *shader_RGB_);
    ReadRGB(rio_data_.color);
    ReadDepth(rio_data_.depth);
    DrawScene(*model_labels_, *shader_labels_);
    ReadLabels(rio_data_.labels, rio_data_.instances);
    if ((save_path != "") && (!rio_data_.color.empty())) {
        std::stringstream filename;
        filename << save_path << "/frame-" << std::setfill('0') << std::setw(6) << data_.frame_id();
        cv::imwrite(filename.str() + ".rendered.color.jpg", rio_data_.color);
        cv::imwrite(filename.str() + ".rendered.depth.png", rio_data_.depth);
        cv::imwrite(filename.str() + ".rendered.labels.png", rio_data_.labels);
        cv::imwrite(filename.str() + ".rendered.instances.png", rio_data_.instances);
        std::ofstream outfile(filename.str() + ".bb.txt");
        for (const auto& bb: rio_data_.bboxes) {
            const Eigen::Vector4i& box = bb.second;
            if (rio_data_.instance2label.find(bb.first) != rio_data_.instance2label.end()) 
                outfile << bb.first << " " << box(0) << " " << box(1) << " " << box(2) << " " << box(3) << std::endl;
        }
        outfile.close();
    }
    if (inc_frame_id)
        data_.NextFrame();
}

void Renderer::DrawScene(Model& model, Shader& shader) {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Render(model, shader);
}

void Renderer::Render(Model& model, Shader& shader) {
    shader.Use();
    Eigen::Matrix4f model_view_projection = projection_ * data_.pose();
    glUniformMatrix4fv(glGetUniformLocation( shader.Program, "model_view_projection"), 1, GL_FALSE, model_view_projection.data());
    Eigen::Matrix4f model_matrix{Eigen::Matrix4f::Identity()};
    model.Draw(shader);
}

void Renderer::InitGLFW() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

bool Renderer::LoadObjects(const std::string& obj_file) {
    std::ifstream is(obj_file);
    std::string dataset((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    std::string err;
    const auto json = json11::Json::parse(dataset, err);
    if (err != "") {
        std::cout << "didn't find objects.json in " << data_path_ << std::endl;
        return false;
    }
    // Iterates through all the scans.
    for (auto &scan_json: json["scans"].array_items()) {
        const std::string& scan_id = scan_json["scan"].string_value();
        if (scan_id != rio_data_.scan_id)
            continue;
        const auto& objects = scan_json["objects"].array_items();
        for (const auto& obj: objects) {
            const int id = std::stoi(obj["id"].string_value());
            const std::string hex_str = obj["ply_color"].string_value();
            unsigned long color_hex;
            std::istringstream iss(hex_str.substr(1));
            iss >> std::hex >> color_hex;
            rio_data_.color2instances[color_hex] = id;
            rio_data_.instance2color[id] = color_hex;
            rio_data_.instance2label[id] = obj["label"].string_value();
        }
    }
    return !rio_data_.color2instances.empty();
}

void Renderer::ReadLabels(cv::Mat& image, cv::Mat& instances) {
    if (!rio_data_.color2instances.empty() || LoadObjects(data_path_ + "/objects.json")) {
        glBindFramebuffer(GL_FRAMEBUFFER, 4);
        image = cv::Mat(buffer_height, buffer_width, CV_8UC3);
        std::vector<float> data_buff(buffer_width * buffer_height * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, buffer_width, buffer_height, GL_RGB, GL_FLOAT, data_buff.data());
        for (int i = 0; i < buffer_height; ++i) {
            for (int j = 0; j < buffer_width; ++j) {
                for (int c = 0; c < 3; c++) {
                    image.at<cv::Vec3b>(buffer_height - i - 1, j)[2 - c] = 
                        static_cast<int>(256 * data_buff[int(3 * i * buffer_width + 3 * j + c)]);
                }
            }
        }
        instances = cv::Mat(buffer_height, buffer_width, CV_16UC1);
        for (int i = 0; i < buffer_width; i++) {
            for (int j = 0; j < buffer_height; j++) {
                const cv::Vec3b& vec = image.at<cv::Vec3b>(j, i);
                const unsigned long color_hex = color_utils::RGB2Hex(vec(2), vec(1), vec(0));
                if (rio_data_.color2instances.find(color_hex) != rio_data_.color2instances.end()) {
                    const unsigned short Id = rio_data_.color2instances[color_hex]; // instance ID
                    // Also intialize set list of bounding boxes.
                    if (rio_data_.bboxes.find(Id) == rio_data_.bboxes.end())
                        rio_data_.bboxes[Id] = Eigen::Vector4i(i, j, i, j);
                    else {
                        rio_data_.bboxes[Id](0) = std::min(i, rio_data_.bboxes[Id](0));
                        rio_data_.bboxes[Id](1) = std::min(j, rio_data_.bboxes[Id](1));
                        rio_data_.bboxes[Id](2) = std::max({i, rio_data_.bboxes[Id](2), rio_data_.bboxes[Id](0)});
                        rio_data_.bboxes[Id](3) = std::max({j, rio_data_.bboxes[Id](3), rio_data_.bboxes[Id](1)});
                    }
                    instances.at<unsigned short>(j, i) = Id;
                } else instances.at<unsigned short>(j, i) = 0;
            }
        }
        cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
        cv::rotate(instances, instances, cv::ROTATE_90_CLOCKWISE);
    }
}

void Renderer::ReadRGB(cv::Mat& image) {
    glBindFramebuffer(GL_FRAMEBUFFER, 4);
    image = cv::Mat(buffer_height, buffer_width, CV_8UC3);
    std::vector<float> data_buff(buffer_width * buffer_height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, buffer_width, buffer_height, GL_RGB, GL_FLOAT, data_buff.data());
    for (int i = 0; i < buffer_height; ++i) {
        for (int j = 0; j < buffer_width; ++j) {
            for (int c = 0; c < 3; c++) {
                image.at<cv::Vec3b>(buffer_height - i - 1, j)[2 - c] = 
                    static_cast<int>(256 * data_buff[int(3 * i * buffer_width + 3 * j + c)]);
            }
        }
    }
    cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
}

void Renderer::ReadDepth(cv::Mat& image) {
    image = cv::Mat(buffer_height, buffer_width, CV_16UC1);
    std::vector<float> data_buff(buffer_height * buffer_width);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, buffer_width, buffer_height, GL_DEPTH_COMPONENT, GL_FLOAT, data_buff.data());
    for (int i = 0; i < buffer_height; i++) {
        for (int j = 0; j < buffer_width; j++) {
            const float zn = (2 * data_buff[static_cast<int>(i * buffer_width + j)] - 1);
            const float ze = (2 * kFarPlane * kNearPlane) / (kFarPlane + kNearPlane + zn*(kNearPlane - kFarPlane));
            image.at<unsigned short>(buffer_height - i - 1, j) = 1000 * ze;
        }
    }
    cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
}

}; // namespace RIO
