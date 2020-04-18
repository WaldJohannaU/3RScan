/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include "data.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "util.h"

namespace RIO {

void Data::NextFrame() {
    frame_id_++;
}

void Data::SetFrame(const int frame) {
    if (frame < poses_.size())
        frame_id_ = frame;
}

Data::Data(const std::string& path): data_path_(path) {
}

void Data::LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose) {
    std::ifstream file(pose_file);
    if (file.is_open()) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                file >> pose(i, j);
        file.close();
    }
}

void Data::LoadViewMatrix() {
    int frame_id = 0;
    Eigen::Matrix4f camera_pose;
    Eigen::Vector3f camera_direction;
    Eigen::Vector3f camera_right;
    Eigen::Vector3f camera_up;
    Eigen::Vector3f camera_eye;
    Eigen::Vector3f camera_center;
    Eigen::Matrix4f view_pose;

    while (true) {
        std::stringstream filename;
        filename << data_path_ << "/" << data_config_.pose_prefix_ << std::setfill('0')
                 << std::setw(6) << frame_id << data_config_.pose_suffix_;
        
        // if file does not exists
        if (!std::ifstream(filename.str()).good()) 
            break;
        LoadPose(filename.str() , camera_pose);
        frame_id++;
        camera_direction = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(0, 0, 1);
        camera_right = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(1, 0, 0);
        camera_up = camera_right.cross(camera_direction);
        camera_eye = camera_pose.block<3, 1>(0, 3);
        camera_center = camera_eye + 1 * camera_direction;

        view_pose = camera_utils::lookAt(camera_eye, camera_center, camera_up);
        poses_.push_back(view_pose);
    }
}

const Eigen::Matrix4f& Data::pose() const {
    return poses_[fmin(static_cast<int>(frame_id_), poses_.size() -1)];
}

const int Data::frame_id() const {
    return frame_id_;
}

bool Data::LoadIntrinsics() {
    std::cout << data_path_ << "/" << data_config_.calib_file_ << std::endl;
    std::string line{""};
    const std::string calib_file = data_path_ + "/" + data_config_.calib_file_;
    std::cout << calib_file << std::endl;
    std::ifstream file(calib_file);
    if (file.is_open()) {
        while (std::getline(file,line)) {
            if (line.rfind("m_colorWidth", 0) == 0)
                intrinsics.width = std::stoi(line.substr(line.find("= ")+2, std::string::npos));
            else if (line.rfind("m_colorHeight", 0) == 0)
                intrinsics.height = std::stoi(line.substr(line.find("= ")+2, std::string::npos));
            else if (line.rfind("m_calibrationColorIntrinsic", 0) == 0) {
                const std::string model = line.substr(line.find("= ")+2, std::string::npos);
                const auto parts = funct_utils::split(model, " ");
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

}; // namespace RIO
