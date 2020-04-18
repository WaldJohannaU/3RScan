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
#include <opencv2/core/core.hpp>

#include "data.h"
#include "frame_config.h"
#include "types.h"

constexpr float kMeterToMillimeter = 1000.0f;
constexpr float kMillimeterToMeter = 0.001f;

class Sequence {
public:
    Sequence(const std::string& data_path, const Data& json_data);
    const Eigen::Matrix4f GetPose(const std::string& scan_id,
                                  const int& frame_id,
                                  const bool normalized2reference,
                                  const bool mm) const;
    const bool Backproject(const std::string& scan_id, const int frame_id, 
                           const bool normalized2reference = false) const;
private:
    const Data& json_data_;
    const FrameConfig config_;
    Eigen::Matrix4f pose{Eigen::Matrix4f::Identity()};
    
    void SaveRIOPlyFile(const std::string file, RIO::PlyData& ply_file, bool ascii) const;
    void LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose,
                  const bool transform_mm) const;
    // Functions to load intrinsics of different types.
    // 3RScan currently only supports _info.txt
    bool LoadIntrinsics(const RIO::CalibFormat& format,
                        const std::string& filename,
                        const bool depth_intrinsics,
                        RIO::Intrinsics& intrinsics) const;
    bool LoadInfoIntrinsics(const std::string& filename,
                            const bool depth_intrinsics,
                            RIO::Intrinsics& intrinsics) const;
    bool LoadYamlIntrinsics(const std::string& filename,
                            RIO::Intrinsics& intrinsics) const;
};
