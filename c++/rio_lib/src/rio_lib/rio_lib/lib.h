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
#include <string>

#include "rio_config.h"

namespace RIO {

class RIOLibInterface {
 public:
    virtual ~RIOLibInterface() {}
    // Returns true if scan_id is a rescan id.
    virtual const bool IsRescan(const std::string& scan_id) const = 0;
    // Returns true if scan_id is a reference scan id.
    virtual const bool IsReference(const std::string& scan_id) const = 0;
    // Returns the reference scan id for a rescan id.
    // Returns an empty string if the provided scan id
    virtual const std::string GetReference(const std::string& scan_id) const = 0;
    // Saves ply (labels) in ASCII format, retruns true if sucessful.
    virtual const bool ReSavePLYASCII(const std::string& scan_id) const = 0;
    // Aligns the rescans 3D model to the reference
    virtual const bool Transform2Reference(const std::string& scan_id) const = 0;
    // Saves ply with remaped local instance id "objectId" to global ID globalId.
    virtual const bool RemapLabelsPly(const std::string& scan_id) const = 0;
    // Prints a list of all the semantic labels of the scan.
    virtual void PrintSemanticLabels(const std::string& scan_id) const = 0;
    // Aligns an instance with the reference scan given the transformation. 
    virtual const bool TransformInstance(const std::string& scan_id, const int& instance) const = 0;
    // Returns the camera pose of frame_id of a given scan_id. The parameter normalize2reference
    // tells if the pose should be returned in the coordinate system of the reference scan
    // If false, the pose is returned in the original rescan coordinate system. 
    virtual const Eigen::Matrix4f GetCameraPose(const std::string& scan_id, const int frame_id,
                                                const bool normalize2reference,
                                                const bool mm = false) const = 0;
    // Backprojects the depth image of a given frame_id with the corresponding camera pose
    // Colores point cloud with the corresponding RGB image.
    virtual const bool Backproject(const std::string& scan_id, const int frame_id,
                                   const bool normalized2reference = false) const = 0;
};

}  // namespace RIO
