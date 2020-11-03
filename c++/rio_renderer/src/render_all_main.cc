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
    if (argc < 4)
        return 0;
    const std::string data_path{argv[1]}; 
    const std::string scan_id{argv[2]}; 
    const std::string output_path{argv[3]}; 
    const std::string seq_path = data_path + "/" + scan_id + "/sequence/";

    bool render_only_occlusion = false;
    float occlusion_fov_scale = 2.0f;
    if (argc == 6){
        render_only_occlusion = atoi(argv[4]);
        occlusion_fov_scale = atof(argv[5]);

        std::cout << "RENDER ONLY OCCLUSION: " << render_only_occlusion << std::endl;
        std::cout << "OCCLUSION FOV SCALE: " << occlusion_fov_scale << std::endl;
    }

    RIO::Renderer renderer(seq_path, data_path, scan_id, !render_only_occlusion, true, occlusion_fov_scale);
    renderer.Init();
    renderer.RenderAllFrames(output_path);
}
