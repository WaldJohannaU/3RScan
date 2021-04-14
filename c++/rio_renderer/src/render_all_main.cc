/*******************************************************
* Copyright (c) 2020, Johanna Wald
* All rights reserved.
*
* This file is distributed under the GNU Lesser General Public License v3.0.
* The complete license agreement can be obtained at:
* http://www.gnu.org/licenses/lgpl-3.0.html
********************************************************/

#include <string>
#include "renderer.h"
#include <opencv2/core/core.hpp>
#include "util.h"

#include <cstdlib>

int main (int argc, char* argv[]) {
    if (argc < 5)
        return 0;
    // data_path scan_id output_folder render_mode
    const std::string data_path{argv[1]}; 
    const std::string scan_id{argv[2]}; 
    const std::string output_folder{argv[3]}; 
    // 0 / default (occlusion) = all, 1 = only images, 2 = only depth
    // 3 = images and bounding boxes, 4 = only bounding boxes
    const int render_mode = atoi({argv[4]}); 
    if (render_mode > 4)
        return 0;

    const std::string seq_path = data_path + "/" + scan_id + "/sequence/";
    const std::string output_path = data_path + "/" + scan_id + "/" + output_folder + "/";

    const bool save_images = (render_mode != 2) && (render_mode != 4);
    const bool save_depth = render_mode != 4;
    const bool save_bounding_boxes = (render_mode != 1) && (render_mode != 2);
    const bool save_occlusion = render_mode == 0;
    const float fov_scale = (save_occlusion ? 2.0f : 1.0f);
    RIO::Renderer renderer(seq_path, data_path, scan_id, save_images, save_depth, save_bounding_boxes, save_occlusion, fov_scale);
    renderer.Init();
    renderer.RenderAllFrames(output_path);
}
