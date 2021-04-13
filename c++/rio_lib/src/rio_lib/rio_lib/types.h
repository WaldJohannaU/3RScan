/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#pragma once

#include <map>
#include <opencv2/core/core.hpp>
#include <string>

namespace RIO {

class Scan {
public:
    // Maps the instance Id to the semantic labels in the current scene.
    std::map<int, std::string> instance2labels;
    // Maps the instance Id of the current scene to the global semantic identifier.
    std::map<int, int> instance2global;
};

enum class CalibFormat { InfoTxt, YAML };

struct PlyData {
    std::vector<float> vertices;
    std::vector<uint8_t> colors;
    const bool save(const std::string& filename, const bool ascii);
};

struct RIOPlyData {
    bool v2 = true;

    std::vector<float> vertices;
    std::vector<uint8_t> colors;
    std::vector<uint32_t> faces;
    std::vector<uint16_t> global_ids;
    std::vector<uint16_t> object_ids;
    // v1 properties
    std::vector<uint16_t> category_ids;
    std::vector<uint8_t> raw_nyu40;
    std::vector<uint8_t> raw_mpr40;
    // v2 properties
    std::vector<uint8_t> NYU40;
    std::vector<uint8_t> Eigen13;
    std::vector<uint8_t> RIO27;

    const bool save(const std::string& filename, const bool ascii);
    const uint32_t load(const std::string filename);
};

struct Intrinsics {
    double fx{0};
    double fy{0};
    double cx{0};
    double cy{0};
    int image_width{0};
    int image_height{0};
};

} // namespace RIO