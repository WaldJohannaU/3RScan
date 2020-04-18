#include "util.h"

namespace color_utils {

unsigned long RGB2Hex(int r, int g, int b) {
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

const Eigen::Vector3i Hex2RGB(unsigned long hexValue) {
    Eigen::Vector3i rgb(0,0,0);
    rgb(0) = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
    rgb(1) = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
    rgb(2) = ((hexValue) & 0xFF);        // Extract the BB byte
    return rgb;
}

}; // namespace color_utils

namespace funct_utils {

std::vector<std::string> split(const std::string& s, const std::string delim) {
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

}; // namespace funct_utils
