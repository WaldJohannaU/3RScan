#include "renderer.h"

#include "json11.hpp"
#include "model.h"
#include "util.h"

namespace RIO {

Renderer::Renderer(const std::string& sequence_path, 
                   const std::string& data_path, 
                   const std::string& scan_id,
                   const bool save_images,
                   const bool save_depth,
                   const bool save_bounding_boxes,
                   const bool save_occlusion,
                   float fov_scale,
                   bool v2): 
                   data_path_(data_path), sequence_path_(sequence_path),
                   rio_data_(scan_id), data_(sequence_path), data_fov_scale_(sequence_path, fov_scale),
                   save_images_(save_images), save_depth_(save_depth),
                   save_bounding_boxes_(save_bounding_boxes),
                   save_occlusion_(save_occlusion), v2_(v2) {
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
    if (!(glfwInit() && data_.LoadIntrinsics() && data_fov_scale_.LoadIntrinsics()))
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

    // load models with all objects in it
    const std::string file_prefix = v2_ ? ".v2" : "";
    model_labels_ = new Model(data_path_ + "/" + rio_data_.scan_id + "/labels.instances.annotated" + file_prefix + ".ply");
    if (save_depth_ || save_images_ || save_bounding_boxes_) {
        model_RGB_ = new Model(data_path_ + "/" + rio_data_.scan_id + "/mesh.refined" + file_prefix + ".obj");
    }

    // load models with just one instance visible
    if (save_occlusion_) {
        LoadObjects(data_path_ + "/objects.json"); // load this now because we need it now (otherwise it is loaded in first render call)
        for (const auto& i2c: rio_data_.instance2color) {
            const int instance_id = i2c.first;
            const unsigned long color = i2c.second; // this is the RGB hex value stored in an ulong
            
            // extract RGB color values
            const int r = ((color >> 16) & 0xFF);  // Extract the RR byte
            const int g = ((color >> 8) & 0xFF);   // Extract the GG byte
            const int b = ((color) & 0xFF);        // Extract the BB byte

            // create model with only the current instance visible
            glm::vec3 rgb(r / 255.0f, g / 255.0f, b / 255.0f); // required to be in [0..1] scale because this is how the colors are defined in the mesh file
            Model* model_label_one_instance_only = new Model(data_path_ + "/" + rio_data_.scan_id + "/labels.instances.annotated" + file_prefix + ".ply", rgb);
            instance2model_with_instance_only_[instance_id] = model_label_one_instance_only;
        }
    }

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
    
    GLuint framebuffername = 1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffername);

    // the default projection matrix (without scaled fov value)
    projection_ = camera_utils::perspective<Eigen::Matrix4f::Scalar>(data_.intrinsics, kNearPlane, kFarPlane);    

    // this is needed for both saving cases so always render it
    DrawScene(*model_labels_, *shader_labels_);
    ReadLabels(rio_data_.labels, rio_data_.instances);
    if (save_path != "" && (save_depth_ || save_images_ || save_bounding_boxes_ || save_occlusion_)) {
        std::stringstream filename;
        filename << save_path << "/frame-" << std::setfill('0') << std::setw(6) << data_.frame_id();

        // render + save rendered color, depth, label, instance images and the bbox file.
        if (save_depth_ || save_images_ || save_bounding_boxes_) {
            // only now are the rgb and depth images needed
            DrawScene(*model_RGB_, *shader_RGB_);
            ReadRGB(rio_data_.color);
            ReadDepth(rio_data_.depth);
            if (save_depth_) {
                if (!cv::imwrite(filename.str() + ".rendered.depth.png", rio_data_.depth))
                    throw std::system_error(errno, std::system_category(), "failed to write " + save_path);
            }
            if (save_images_) {
                if (!cv::imwrite(filename.str() + ".rendered.color.jpg", rio_data_.color) |
                    !cv::imwrite(filename.str() + ".rendered.labels.png", rio_data_.labels) |
                    !cv::imwrite(filename.str() + ".rendered.instances.png", rio_data_.instances))
                    throw std::system_error(errno, std::system_category(), "failed to write " + save_path);
            }
            if (save_bounding_boxes_) {
                std::ofstream outfile(filename.str() + ".bb.txt");
                for (const auto& bb: rio_data_.bboxes) {
                    const Eigen::Vector4i& box = bb.second;
                    if (rio_data_.instance2label.find(bb.first) != rio_data_.instance2label.end()) 
                        outfile << bb.first << " " << box(0) << " " << box(1) << " " << box(2) << " " << box(3) << std::endl;
                }
                outfile.close();
            }
        }

        // render + save the occlusion score for each object instance id
        if (save_occlusion_) {
            // use original intrinsics for calculating occlusion
            CalcOcclusions(rio_data_.instance2color, rio_data_.instances2occlusion, rio_data_.labels);

            // use fov scaled intrinsics for calcuating truncation
            projection_ = camera_utils::perspective<Eigen::Matrix4f::Scalar>(data_fov_scale_.intrinsics, kNearPlane, kFarPlane);
            DrawScene(*model_labels_, *shader_labels_);
            ReadLabels(rio_data_.labels_fov_scale, rio_data_.instances);
            CalcTruncations(rio_data_.instance2color, rio_data_.instances2truncation, rio_data_.labels_fov_scale);

            // verify calculations
            VerifyTruncationAndOcclusion(rio_data_.instances2occlusion, rio_data_.instances2truncation);

            std::ofstream outfile(filename.str() + ".visibility.txt");
            for (const auto& instance2occlusion : rio_data_.instances2occlusion) {
                int instance_id = instance2occlusion.first;
                Renderer::VisibilityEntry occlusion = instance2occlusion.second;
                Renderer::VisibilityEntry truncation = rio_data_.instances2truncation[instance_id];
                
                if (rio_data_.instance2label.find(instance_id) != rio_data_.instance2label.end())
                    outfile << instance_id << " " << truncation.original_pixel_count << " " << truncation.complete_pixel_count << " " << truncation.ratio << " " << occlusion.original_pixel_count << " " << occlusion.complete_pixel_count << " " << occlusion.ratio << std::endl;
            }
            outfile.close();
        }
        
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
                        static_cast<int>(255 * data_buff[int(3 * i * buffer_width + 3 * j + c)]);
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
                    static_cast<int>(255 * data_buff[int(3 * i * buffer_width + 3 * j + c)]);
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

void Renderer::CalcTruncations(std::map<int, unsigned long>& instances2color, std::map<int, Renderer::VisibilityEntry>& instances2truncation, const cv::Mat& labels_fov_scale) {
    
    for (const auto& instance2color: instances2color) {
        const int instance_id = instance2color.first;
        const unsigned long color = instance2color.second; // this is the RGB hex value stored in an ulong

        // check if the parsing worked as expected. This should only fail when something is wrong with the metadata.
        if (rio_data_.instance2label.find(instance_id) == rio_data_.instance2label.end()) continue;

        // extract RGB color values
        const int r = ((color >> 16) & 0xFF);  // Extract the RR byte
        const int g = ((color >> 8) & 0xFF);   // Extract the GG byte
        const int b = ((color) & 0xFF);        // Extract the BB byte

        // create rect at large image to crop small image part of it
        const int rows = labels_fov_scale.rows;
        const int cols = labels_fov_scale.cols;

        // the original image is located at the center of the rendered image with below width/height.
        // Its upper left corner is thus fov_scale*2-times the width/height (== center of rendered image - half of original image width/height)
        const int x = static_cast<int>(rows / (data_fov_scale_.fov_scale()*2));
        const int y = static_cast<int>(cols / (data_fov_scale_.fov_scale()*2));

        // the original image is fov_scale-times smaller than the rendered image
        const int width = static_cast<int>(rows / data_fov_scale_.fov_scale());
        const int height = static_cast<int>(cols / data_fov_scale_.fov_scale());
        cv::Rect rect(y, x, height, width);
        cv::Mat original_image = labels_fov_scale(rect);

        // create image masks that only contain the color
        cv::Mat dst_large, dst_small;
        cv::Scalar cv_color(b, g, r); // color must be in BGR format

        cv::inRange(labels_fov_scale, cv_color, cv_color, dst_large); // creates a black-white image in dst that has white pixels that match the color, rest black
        const int number_pixels_large = cv::countNonZero(dst_large); // counts the white pixels

        cv::inRange(original_image, cv_color, cv_color, dst_small); // creates a black-white image in dst that has white pixels that match the color, rest black
        const int number_pixels_small = cv::countNonZero(dst_small); // counts the white pixels

        // instance2color map contains all instances for the scene and not all instances for this frame.
        // Thus, some instances are not visible in the current frame or frame-enlargement. 
        // If this is the case, we do not consider that instance to have a truncation score in this frame.
        // Only instances that are visible will be added to the truncation list for this frame.
        const bool valid_instance = number_pixels_large > 0 && number_pixels_small > 0; 

        if (valid_instance) {
            instances2truncation[instance_id] = Renderer::VisibilityEntry(static_cast<float>(number_pixels_small),
                                                                          static_cast<float>(number_pixels_large));
        }
    }
    
}

void Renderer::CalcOcclusions(std::map<int, unsigned long>& instances2color, std::map<int, Renderer::VisibilityEntry>& instances2occlusion, const cv::Mat& labels){
    for (const auto& instance2color: instances2color) {
        const int instance_id = instance2color.first;
        const unsigned long color = instance2color.second; // this is the RGB hex value stored in an ulong

        // check if the parsing worked as expected. This should only fail when something is wrong with the metadata.
        if (rio_data_.instance2label.find(instance_id) == rio_data_.instance2label.end()) continue;

        // extract RGB color values
        const int r = ((color >> 16) & 0xFF);  // Extract the RR byte
        const int g = ((color >> 8) & 0xFF);   // Extract the GG byte
        const int b = ((color) & 0xFF);        // Extract the BB byte

        // load model with only the current instance visible
        Model* model_label_one_instance_only = instance2model_with_instance_only_[instance_id];

        // render label image with just this instance
        cv::Mat label_image_one_instance_only, instance_image_one_instance_only;
        DrawScene(*model_label_one_instance_only, *shader_labels_);
        ReadLabels(label_image_one_instance_only, instance_image_one_instance_only);

        // count the pixels in the original image (with all instances rendered) and in the image with just this instance
        cv::Mat dst_original, dst_one_instance_only;
        cv::Scalar cv_color(b, g, r); // color must be in BGR format

        cv::inRange(labels, cv_color, cv_color, dst_original); // creates a black-white image in dst that has white pixels that match the color, rest black
        const int number_pixels_original = cv::countNonZero(dst_original); // counts the white pixels

        cv::inRange(label_image_one_instance_only, cv_color, cv_color, dst_one_instance_only); // creates a black-white image in dst that has white pixels that match the color, rest black
        const int number_pixels_one_instance_only = cv::countNonZero(dst_one_instance_only); // counts the white pixels

        // instance2color map contains all instances for the scene and not all instances for this frame.
        // Thus, some instances are not visible in the current frame or frame-enlargement. 
        // If this is the case, we do not consider that instance to have a occlusion score in this frame.
        // Only instances that are visible will be added to the occlusion list for this frame.
        const bool valid_instance = number_pixels_original > 0 && number_pixels_one_instance_only > 0; 

        if(valid_instance){
            instances2occlusion[instance_id] = Renderer::VisibilityEntry(static_cast<float>(number_pixels_original),
                                                                            static_cast<float>(number_pixels_one_instance_only));
        }
    }
}

void Renderer::VerifyTruncationAndOcclusion(std::map<int, Renderer::VisibilityEntry>& instances2occlusion, std::map<int, Renderer::VisibilityEntry>& instances2truncation){
    // create dummy vector to add in all cases where no value is found
    Renderer::VisibilityEntry dummy_value;
    
    // go over all entries in occlusion and check if truncation is also set. If it is not set: set it to the dummy_value
    for(auto it = instances2occlusion.cbegin(); it != instances2occlusion.cend(); ++it) {
        bool present = instances2truncation.find(it->first) != instances2truncation.end();
        if(! present){
            instances2truncation[it->first] = dummy_value;
        }
    }

    // go over all entries in truncation and check if occlusion is also set. If it is not set: set it to the dummy_value
    for(auto it = instances2truncation.cbegin(); it != instances2truncation.cend(); ++it) {
        bool present = instances2occlusion.find(it->first) != instances2occlusion.end();
        if(! present){
            instances2occlusion[it->first] = dummy_value;
        }
    }

    // now the size must be equal otherwise this is a huge error
    assert(rio_data_.instances2truncation.size() == rio_data_.instances2occlusion.size());
}

void Renderer::RenderAllFrames(const std::string save_path) {
    data_.SetFrame(0);
    while (data_.HasNextFrame()){    
        Render(true, save_path);
        // Clear frame-specific data to be re-evaluated in the next render pass for the next frame.
        rio_data_.bboxes.clear();
        rio_data_.instances2truncation.clear();
        rio_data_.instances2occlusion.clear();
        
        // These would not really need to be cleared because currently we save all instances of the scene in each Render pass and not all instances of the scene that are visible only in the current frame.
        // rio_data_.instance2label.clear();
        // rio_data_.instance2color.clear();
        // rio_data_.color2instances.clear();
    }
}

}; // namespace RIO
