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
#include <map>
#include <string>

#include "third_party/json11.hpp"

typedef std::map<int, Eigen::Matrix4f, std::less<int>,
        Eigen::aligned_allocator<std::pair<const int, Eigen::Matrix4f>>> MapIntMatrix4fAligned;

class Data {
private:
    // Maps a scan id to the corresponding reference id
    std::map<std::string, std::string> scan2references{};
    class ReScan {
    public:
        // scan id of the corresponding reference.
        std::string reference_id{""};
        // Transformation that aligns the rescan with the reference.
        Eigen::Matrix4f rescan2reference{Eigen::Matrix4f::Identity()};
        MapIntMatrix4fAligned rigid_transforms{};
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
    
    // List of rescans identified by the scan id (key).
    std::map<std::string, ReScan> rescans{};
    void ReadJson(const std::string& data_file);
    void ReadRescanJson(const std::string& scan_id,
                        const std::string& reference_id,
                        const json11::Json& json_object);
    void ReadMatrix(Eigen::Matrix4f& matrix, const json11::Json& json_matrix);
public:
    Data(const std::string& data_file);
    
    const std::string GetReference(const std::string& scan_id) const;
    const bool IsRescan(const std::string& scan_id) const;
    const bool IsReference(const std::string& scan_id) const;
    const Eigen::Matrix4f GetRigidTransform(const std::string& scan_id, const int& instance) const;
    const Eigen::Matrix4f GetRescanTransform(const std::string& scan_id) const;
    const std::map<std::string, std::string>& GetScan2References() const;
};
