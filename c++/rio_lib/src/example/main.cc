/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include <iostream>
#include <rio_lib/rio_config.h>
#include <rio_lib/rio.h>

int main(int argc, char **argv) {
    if (argc < 3)
        return 0;
    const std::string data_path{argv[1]}; 
    const std::string scan_id{argv[2]}; 
    const RIO::RIOConfig config(data_path);
    RIO::RIO rio(config);
    // check if a scan id is a rescan
    const bool is_rescan = rio.IsRescan(scan_id);
    // check if a scan id is a reference
    const bool is_reference = rio.IsReference(scan_id);
    // get reference scan id for a given rescan id
    const std::string reference_id = rio.GetReference(scan_id);
    // re-save binary encoded labels.instances.annotated.ply as ASCII file
    // this creates a labels.instances.annotated.ascii.ply in data_path/scan_id
    rio.ReSavePLYASCII(scan_id);
    // transforms *.obj and *.ply to be aligned to the reference.
    if (is_rescan)
        rio.Transform2Reference(scan_id);
    // saves ply with remaped local instance id "objectId" to global ID globalId.
    rio.RemapLabelsPly(scan_id);
    // Prints semantic labels:
    rio.PrintSemanticLabels(scan_id);
    // Transforms Instance 10 to the reference given the ground truth transformation.
    if (argc > 3)
        rio.TransformInstance(scan_id, std::stoi(argv[3]));
    // Return the camera pose
    const Eigen::Matrix4f& pose = rio.GetCameraPose(scan_id, 0, false);
    const Eigen::Matrix4f& pose_normalized = rio.GetCameraPose(scan_id, 0, true);
    std::cout << pose << std::endl << std::endl << pose_normalized << std::endl;
    // Backproject depth image and color
    rio.Backproject(scan_id, 28, true);
    return 0;
}
