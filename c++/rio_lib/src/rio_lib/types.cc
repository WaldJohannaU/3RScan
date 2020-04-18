/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include "rio_lib/types.h"

#include "third_party/tinyply.h"

#include <fstream>

namespace RIO {

const bool PlyData::save(const std::string& filename, const bool ascii) {
    std::filebuf fb;
    fb.open(filename, std::ios::out | std::ios::binary);
    std::ostream ss(&fb);
    tinyply::PlyFile out_file;
    out_file.add_properties_to_element("vertex", { "x", "y", "z" }, vertices);
    out_file.add_properties_to_element("vertex", { "red", "green", "blue" }, colors);
    out_file.write(ss, !ascii);
    fb.close();
    std::cout << "saved as " << filename << std::endl;
    return true;
}

const bool RIOPlyData::save(const std::string& filename, const bool ascii) {
    std::filebuf fb;
    fb.open(filename, std::ios::out | std::ios::binary);
    std::ostream ss(&fb);
    tinyply::PlyFile out_file;
    out_file.add_properties_to_element("vertex", { "x", "y", "z" }, vertices);
    out_file.add_properties_to_element("vertex", { "red", "green", "blue" }, colors);
    out_file.add_properties_to_element("vertex", { "objectId" }, object_ids);
    out_file.add_properties_to_element("vertex", { "categoryId" }, category_ids);
    out_file.add_properties_to_element("vertex", { "NYU40" }, raw_nyu40);
    out_file.add_properties_to_element("vertex", { "mpr40" }, raw_mpr40);
    if (!global_ids.empty())
        out_file.add_properties_to_element("vertex", { "globalId" }, global_ids);
    out_file.add_properties_to_element("face", { "vertex_indices" }, faces, 3, tinyply::PlyProperty::Type::UINT8);
    out_file.write(ss, !ascii);
    fb.close();
    std::cout << "saved as " << filename << std::endl;
    return true;
}

const uint32_t RIOPlyData::load(const std::string filename) {
    std::ifstream ss(filename, std::ios::binary);
    tinyply::PlyFile input_file(ss);
    input_file.request_properties_from_element("vertex", { "x", "y", "z" }, vertices);
    input_file.request_properties_from_element("vertex", { "red", "green", "blue" }, colors);
    input_file.request_properties_from_element("vertex", { "objectId" }, object_ids);
    input_file.request_properties_from_element("vertex", { "categoryId" }, category_ids);
    input_file.request_properties_from_element("vertex", { "NYU40" }, raw_nyu40);
    input_file.request_properties_from_element("vertex", { "mpr40" }, raw_mpr40);
    input_file.request_properties_from_element("face", { "vertex_indices" }, faces, 3);
    input_file.read(ss);
    return vertices.size()/3;
}

} // namespace RIO