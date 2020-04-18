/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#pragma once

#include <Eigen/Core>

namespace utils {

const std::vector<std::string> split(const std::string s, const std::string delim) {
    std::vector<std::string> list;
    auto start = 0U;
    auto end = s.find(delim);
    while (true) {
        list.push_back(s.substr(start, end - start));
        if (end == std::string::npos)
            break;
        start = end + delim.length();
        end = s.find(delim, start);
    }
    return list;
}

const std::string frame2str(const int frame_id) {
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << frame_id << std::endl;
    return ss.str();
}

template<class T>
const Eigen::Matrix<T,4,4> rotation_matrix_Z(const float rot = M_PI) {
    Eigen::Matrix<T,4,4> res = Eigen::Matrix<T,4,4>::Identity();
    res << cos(rot), -sin(rot), 0, 0,
            sin(rot), cos(rot), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;
    return res;
}

};
