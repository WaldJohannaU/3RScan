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

#include <vector>

#include "intrinsics.h"

namespace color_utils {

unsigned long RGB2Hex(int r, int g, int b);
const Eigen::Vector3i Hex2RGB(unsigned long hexValue);

}; // namespace color_utils

namespace funct_utils {

std::vector<std::string> split(const std::string& s, const std::string delim);

}; // namespace funct_utils

namespace camera_utils {

template<class T>
Eigen::Matrix<T,4,4> perspective(const RIO::Intrinsics& intr, double n, double f) {
    assert(f > n);
    Eigen::Matrix<T,4,4> res = Eigen::Matrix<T,4,4>::Zero();
    res << 2 * intr.fx / intr.width, 0, -(2*(intr.cx / intr.width) -1), 0,
           0, 2 * intr.fy / intr.height, -(2*(intr.cy / intr.height) -1), 0,
           0, 0,  -(f+n)/(f-n), -2*f*n/(f-n),
           0, 0, -1, 0;
    return res;
}

template<class T>
Eigen::Matrix<T,4,4> lookAt(Eigen::Matrix<T,3,1> const& eye,
                            Eigen::Matrix<T,3,1> const& center,
                            Eigen::Matrix<T,3,1> const & up) {
    const Eigen::Matrix<T,3,1> f = (center - eye).normalized();
    Eigen::Matrix<T,3,1> u = up.normalized();
    const Eigen::Matrix<T,3,1> s = f.cross(u).normalized();
    u = s.cross(f);
    Eigen::Matrix<T,4,4> res;
    res <<  s.x(), s.y(), s.z(), -s.dot(eye),
            u.x(), u.y(), u.z(), -u.dot(eye),
            -f.x(), -f.y(), -f.z(), f.dot(eye),
            0, 0, 0, 1;
    return res;
};

}; // namespace camera_utils
