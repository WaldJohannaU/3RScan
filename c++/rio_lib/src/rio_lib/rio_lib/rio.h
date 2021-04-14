/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#pragma once

#include <cstdint>

#include "data.h"
#include "data_config.h"
#include "lib.h"
#include "rio_config.h"
#include "sequence.h"
#include "types.h"

namespace RIO {

// We have 537 different global classes.
constexpr int16_t kglobalId_size = 537;

// Color range for the global instance id.
constexpr int16_t color_range_from = 80;
constexpr int16_t color_range_to = 256;

class RIO: public RIOLibInterface { 
public:
    RIO(const RIOConfig& config);
    // Get the reference scan id of the given rescan.
    const std::string GetReference(const std::string& scan_id) const override;
    // Returns true if the given scan_id is a rescan.
    const bool IsRescan(const std::string& scan_id) const override;
    // Returns true if the given scan_id is a reference.
    const bool IsReference(const std::string& scan_id) const override;
    // re-save binary encoded labels.instances.annotated.ply as ASCII file
    // this creates a labels.instances.annotated.ascii.ply in data_path/scan_id
    const bool ReSavePLYASCII(const std::string& scan_id) const override;
    // aligns labels.instances.annotated.ply and mesh.refined.obj of a rescan
    // to the reference and creates labels.instances.annotated.align.ply and
    // mesh.refined.align.obj in data_path/scan_id
    const bool Transform2Reference(const std::string& scan_id) const override;
    // saves ply with remaped local instance id "objectId" to global ID globalId.
    const bool RemapLabelsPly(const std::string& scan_id) const override;
    // Returns the camera pose of frame_id of a given scan_id. The parameter normalize2reference
    // tells if the pose should be returned in the coordinate system of the reference scan
    // If false, the pose is returned in the original rescan coordinate system. 
    const Eigen::Matrix4f GetCameraPose(const std::string& scan_id, 
                                        const int frame_id,
                                        const bool normalize2reference,
                                        const bool mm = false) const override;
    // Sets the camera pose of frame_id of a given scan_id. Behaves the same as
    // Eigen::Matrix4f GetCameraPose() but returns false if reading the camera pose file was
    // unsecessful.
    const bool GetCameraPose(Eigen::Matrix4f& pose,
                             const std::string& scan_id, 
                             const int frame_id,
                             const bool normalize2reference,
                             const bool mm = false) const override;
    // Backprojects the depth image of a given frame_id with the corresponding camera pose
    // Colores point cloud with the corresponding RGB image.
    const bool Backproject(const std::string& scan_id, const int frame_id, 
                           const bool normalized2reference = false) const override;
    // Prints a list of all the semantic labels of the scan.
    void PrintSemanticLabels(const std::string& scan_id) const override;
    const bool TransformInstance(const std::string& scan_id, const int& instance) const override;
private:
    const bool LoadObjects(const std::string& objects);
    bool TransformPly2Reference(const std::string& scan_id,
                                const std::string& filename_in,
                                const std::string& filename_out) const;
    bool TransformObj2Reference(const std::string& scan_id,
                                const std::string& filename_in,
                                const std::string& filename_out) const;
    // bool ReSaveObjInstance(const std::string& scan_id, const int& instance) const;
    // void AlignModels2Scene() const;
    // Semantic data of the scans:
    std::map<std::string, Scan> scans;
    
    void InitGlobalId2Color(const int size);
    std::vector<Eigen::Vector3i, Eigen::aligned_allocator<Eigen::Vector3i>> globalId2color;

    const RIOConfig config_;
    const DataConfig data_config_;
    Data json_data_;
    const Sequence sequence_;
};

}  // namespace RIO
