#include "rio_lib/sequence.h"

#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "rio_lib/utils.h"
#include "rio_lib/types.h"

Sequence::Sequence(const std::string& data_path, const Data& json_data): json_data_(json_data), config_(data_path) {
}

const Eigen::Matrix4f Sequence::GetPose(const std::string& scan_id, 
                                        const int& frame_id,
                                        const bool normalized2reference,
                                        const bool mm) const {
    Eigen::Matrix4f pose{Eigen::Matrix4f::Identity()};
    LoadPose(config_.GetPose(scan_id, frame_id), pose, mm);
    if (normalized2reference) {
        Eigen::Matrix4f rescan2reference{json_data_.GetRescanTransform(scan_id)};
        if (mm)
            rescan2reference.block<3,1>(0,3) *= kMeterToMillimeter;
        return rescan2reference * pose;
    }
    else
         return pose;
}

void Sequence::LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose,
                        const bool transform_mm) const {
    // Read camera pose (it's the transformation from the camera to the world coordiante system).
    std::ifstream file(pose_file);
    if (file.is_open()) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                file >> pose(i, j);
        if (transform_mm)
            pose.block<3,1>(0,3) *= kMeterToMillimeter;
        file.close();
    }
}

bool Sequence::LoadInfoIntrinsics(const std::string& filename,
                                  const bool depth_intrinsics,
                                  RIO::Intrinsics& intrinsics) const {
    const std::string search_tag = depth_intrinsics ? "m_calibrationDepthIntrinsic" : "m_calibrationColorIntrinsic";
    /*
     m_colorWidth = 960
     m_colorHeight = 540
     m_depthWidth = 224
     m_depthHeight = 172
     m_depthShift = 1000
     m_calibrationColorIntrinsic = 756.832 0 492.889 0 0 756.026 270.419 0 0 0 1 0 0 0 0 1
     m_calibrationColorExtrinsic = 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1
     m_calibrationDepthIntrinsic = 176.594 0 114.613 0 0 240.808 85.7915 0 0 0 1 0 0 0 0 1
     m_calibrationDepthExtrinsic = 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1
     m_frames.size = 467
    */
    std::string line{""};
    std::ifstream file(filename);
    if (file.is_open()) {
        while (std::getline(file,line)) {
            if (line.rfind("m_colorWidth", 0) == 0)
                intrinsics.image_width = std::stoi(line.substr(line.find("= ")+2, std::string::npos));
            else if (line.rfind("m_colorHeight", 0) == 0)
                intrinsics.image_height = std::stoi(line.substr(line.find("= ")+2, std::string::npos));
            else if (line.rfind(search_tag, 0) == 0) {
                const std::string model = line.substr(line.find("= ")+2, std::string::npos);
                const auto parts = utils::split(model, " ");
                intrinsics.fx = std::stof(parts[0]);
                intrinsics.fy = std::stof(parts[5]);
                intrinsics.cx = std::stof(parts[2]);
                intrinsics.cy = std::stof(parts[6]);
            }
        }
        file.close();
        return true;
    }
    return false;
}

bool Sequence::LoadYamlIntrinsics(const std::string& filename,
                                  RIO::Intrinsics& intrinsics) const {
    std::string line{""};
    std::ifstream file(filename);
    if (file.is_open()) {
        while (std::getline(file,line)) {
            if (line.rfind("  model: ", 0) == 0) {
                const std::string model = line.substr(9, std::string::npos);
                const auto start = model.find('[');
                const auto end = model.find(']');
                const std::string s = model.substr(start + 1, end - 1);
                const auto parts = utils::split(s, ",");
                intrinsics.fx = std::stof(parts[0]);
                intrinsics.fy = std::stof(parts[1]);
                intrinsics.cx = std::stof(parts[3]);
                intrinsics.cy = std::stof(parts[2]);
            } else {
                if (line.rfind("  width: ", 0) == 0) {
                    intrinsics.image_height = std::stoi(line.substr(9, std::string::npos));
                } else if (line.rfind("  height: ", 0) == 0) {
                    intrinsics.image_width = std::stoi(line.substr(10, std::string::npos));
                }
            }
        }
        file.close();
        return true;
    }
    return false;
}

bool Sequence::LoadIntrinsics(const RIO::CalibFormat& format,
                              const std::string& filename,
                              const bool depth_intrinsics,
                              RIO::Intrinsics& intrinsics) const {
    if (format == RIO::CalibFormat::InfoTxt)
        return LoadInfoIntrinsics(filename, depth_intrinsics, intrinsics);
    else
        return LoadYamlIntrinsics(filename, intrinsics);
    return false;
}

const bool Sequence::Backproject(const std::string& scan_id, const int frame_id,
                                 const bool normalized2reference) const {
    std::cout << normalized2reference << std::endl;
    // The depth stores the distance in mm as a 16bit.
    cv::Mat RGB_resized;
    cv::Mat depth = cv::imread(config_.GetDepth(scan_id, frame_id), -1);
    if (depth.empty()) {
        std::cout << "file not found." << config_.GetDepth(scan_id, frame_id) << std::endl;
        return false;
    }
    cv::Mat RGB = cv::imread(config_.GetColor(scan_id, frame_id), -1);
    cv::resize(RGB, RGB_resized, cv::Size(depth.cols, depth.rows));
    Eigen::Matrix4f pose_camera2world = GetPose(scan_id, frame_id, normalized2reference, true);
    // Load intrinsics
    RIO::Intrinsics intrinsics;
    if (LoadIntrinsics(RIO::CalibFormat::InfoTxt, config_.GetCameraInfo(scan_id), true, intrinsics)) {
        RIO::PlyData ply_data;
        for (int row = 0; row < depth.rows; row++) {
            for (int col = 0; col < depth.cols; col++) {
                // the depth
                const unsigned short depth_value = depth.at<unsigned short>(row, col);
                if (depth_value != 0) {
                    // the color pixel at (row, col) corresponds to the depth pixel (row, col).
                    const cv::Vec3b color = RGB_resized.at<cv::Vec3b>(row, col);
                    // backproject 2D depth
                    const Eigen::Vector2f xy((col - intrinsics.cx) / intrinsics.fx, (row - intrinsics.cy) / intrinsics.fy);
                    const Eigen::Vector4f point(xy.x() * depth_value, xy.y() * depth_value, depth_value, 1.0f);
                    // apply pose transformation
                    const Eigen::Vector4f point_transformed = pose_camera2world * point;
                    ply_data.vertices.push_back(point_transformed(0) * kMillimeterToMeter);
                    ply_data.vertices.push_back(point_transformed(1) * kMillimeterToMeter);
                    ply_data.vertices.push_back(point_transformed(2) * kMillimeterToMeter);

                    ply_data.colors.push_back(color(2));
                    ply_data.colors.push_back(color(1));
                    ply_data.colors.push_back(color(0));
                }
            }
        }
        ply_data.save(config_.GetPly(scan_id, frame_id), true);
        return !ply_data.vertices.empty();
    }
    return false;
}

