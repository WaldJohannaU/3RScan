/*******************************************************
* Copyright (c) 2020, Johanna Wald
* All rights reserved.
*
* This file is distributed under the GNU Lesser General Public License v3.0.
* The complete license agreement can be obtained at:
* http://www.gnu.org/licenses/lgpl-3.0.html
********************************************************/

#pragma once

namespace RIO {

struct Intrinsics {
    double fx{0};
    double fy{0};
    double cx{0};
    double cy{0};
    int width{0};
    int height{0};
};

}; // namespace RIO
