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

int main (int argc, char* argv[]) {
    if (argc < 3)
        return 0;
    const std::string data_path{argv[1]}; 
    const std::string scan_id{argv[2]}; 
    const std::string output_folder{argv[3]}; 
    const std::string seq_path = data_path + "/" + scan_id + "/sequence";
    const std::string output_path = data_path + "/" + scan_id + "/" + output_folder;
    RIO::Renderer renderer(seq_path, data_path, scan_id);
    renderer.Init();
    renderer.Render(28, output_path);
    // let's visualize the bounding boxes
    cv::Mat bb_image;
    renderer.GetColor().copyTo(bb_image);
    if (!bb_image.empty()) {
        cv::rotate(bb_image, bb_image, cv::ROTATE_90_COUNTERCLOCKWISE);
        const std::map<int, Eigen::Vector4i>& bboxes = renderer.Get2DBoundingBoxes();
        const std::map<int, std::string>& id2label = renderer.GetInstance2Label();
        const std::map<int, unsigned long>& id2color = renderer.GetInstance2Color();
        for (const auto& bb: bboxes) {
            const Eigen::Vector4i& box = bb.second;
            if (id2label.find(bb.first) != id2label.end()) {
                const Eigen::Vector3i rgb = color_utils::Hex2RGB(id2color.at(bb.first));
                cv::rectangle(bb_image, cv::Point(box(0), box(1)), cv::Point(box(2), box(3)), cv::Scalar(rgb(2),rgb(1),rgb(0)), 3);
            }
        }
        return 0;
        cv::resize(bb_image, bb_image, cv::Size(bb_image.cols/4, bb_image.rows/4));
        cv::rotate(bb_image, bb_image, cv::ROTATE_90_CLOCKWISE);
        cv::imshow("Bounding Boxes", bb_image);
        cv::waitKey(0);
    }
}
