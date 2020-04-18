/*******************************************************
* Copyright (c) 2020, Johanna Wald
* All rights reserved.
*
* This file is distributed under the GNU Lesser General Public License v3.0.
* The complete license agreement can be obtained at:
* http://www.gnu.org/licenses/lgpl-3.0.html
********************************************************/

#pragma once

#include <Eigen/Dense>
#include <fstream>
#include <vector>

#include "intrinsics.h"

namespace RIO {

class Data {
public:
    Data(const std::string& path);
    Intrinsics intrinsics;

    void LoadViewMatrix();
    bool LoadIntrinsics();
    void NextFrame();
    void SetFrame(const int frame_id);

    const Eigen::Matrix4f& pose() const;
    const int frame_id() const;
private:
    const std::string data_path_{""};
    int frame_id_{0};
    struct DataConfig {
        const std::string calib_file_{"_info.txt"};
        const std::string pose_prefix_{"frame-"};
        const std::string pose_suffix_{".pose.txt"};
    };
    DataConfig data_config_;
    std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> poses_;
    void LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose);
};

}; // namespace RIO
