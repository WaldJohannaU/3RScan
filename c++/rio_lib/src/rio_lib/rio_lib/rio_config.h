/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#pragma once

#include <string>

namespace RIO {

struct RIOConfig {
    const std::string data_path{""};
    RIOConfig(const RIOConfig& config): data_path(config.data_path) { }
    RIOConfig(const std::string data_path): data_path(data_path) { }
};

}  // namespace rio
