/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#pragma once

#include <iomanip>
#include <iostream>
#include <string>

struct FrameConfig {
    const std::string base_path{""};
    const std::string sequence_zip{"sequence.zip"};
    const std::string sequence_folder{"sequence"};
    const std::string frame_prefix{"frame-"};
    const std::string frame_color_suffix{".color.jpg"};
    const std::string frame_pose_suffix{".pose.txt"};
    const std::string frame_depth_suffix{".depth.pgm"};
    const std::string frame_ply_suffix{".cloud.ply"};
    
    const std::string camera_info{"_info.txt"};
    const std::string camera_yaml{"camera.yaml"}; 

    FrameConfig(const std::string& base_path): base_path(base_path) { }

    const std::string GetCameraInfo(const std::string& scan_id) const {
        return base_path + "/" + scan_id + "/" + sequence_folder + "/" + camera_info;
    }

    const std::string GetCameraYaml(const std::string& scan_id) const {
        return base_path + "/" + scan_id + "/" + sequence_folder + "/" + camera_yaml;
    }
    
    const std::string GetPath(const std::string& scan_id, const int frame_id) const {
        std::stringstream ss;
        ss << base_path << "/" << scan_id << "/" << sequence_folder << "/"
           << frame_prefix << std::setw(6) << std::setfill('0') << frame_id;
        return ss.str();
    }
    
    const std::string GetPly(const std::string& scan_id, const int frame_id) const {
        return GetPath(scan_id, frame_id) + frame_ply_suffix;
    }
    
    const std::string GetDepth(const std::string& scan_id, const int frame_id) const {
        return GetPath(scan_id, frame_id) + frame_depth_suffix;
    }

    const std::string GetPose(const std::string& scan_id, const int frame_id) const {
        return GetPath(scan_id, frame_id) + frame_pose_suffix;
    }
    
    const std::string GetColor(const std::string& scan_id, const int frame_id) const {
        return GetPath(scan_id, frame_id) + frame_color_suffix;
    }
};
