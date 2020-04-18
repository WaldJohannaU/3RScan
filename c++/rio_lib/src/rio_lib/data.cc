// dataset loader

#include <fstream>
#include <iostream>

#include "rio_lib/data.h"

Data::Data(const std::string& data_file) {
    std::cout << "loading " << data_file << std::endl;
    ReadJson(data_file);
}

void Data::ReadJson(const std::string& data_file) {
    // Read json file and save its output.
    std::ifstream is(data_file);
    std::string dataset((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    std::string err;
    const auto json = json11::Json::parse(dataset, err);
    if (err != "") {
        std::cerr << "Error reading " << data_file << " " << err << std::endl;
        return;
    }
    for (auto &s: json.array_items()) {
        const auto scans = s["scans"].array_items();
        const std::string& reference_id = s["reference"].string_value();
        const std::string& type = s["type"].string_value();
        for (auto &scan: scans) {
            const std::string& scan_id = scan["reference"].string_value();
            scan2references[scan_id] = reference_id;
            if (type != "test")
                ReadRescanJson(scan_id, reference_id, scan);
        }
    }
}

void Data::ReadMatrix(Eigen::Matrix4f& matrix, const json11::Json& json_matrix) {
    matrix = Eigen::Matrix4f::Identity();
    int i = 0;
    for (auto& elem: json_matrix.array_items())
        matrix(i++) = elem.number_value();
}

void Data::ReadRescanJson(const std::string& scan_id,
                          const std::string& reference_id,
                          const json11::Json& json_object) {
    // Read Rescan scan data.
    ReScan rescan;
    rescan.reference_id = reference_id;
    ReadMatrix(rescan.rescan2reference, json_object["transform"]);
    for (const auto& rigid: json_object["rigid"].array_items()) {
        const int instance_id = rigid["instance_reference"].number_value();
        Eigen::Matrix4f rigid_transform;
        ReadMatrix(rigid_transform, rigid["transform"]);
        // The transformation in the json is actually from reference to rescan
        // so we need to take the inverse.
        rescan.rigid_transforms[instance_id] = rigid_transform.inverse();
    }
    rescans[scan_id] = rescan;
}

const std::string Data::GetReference(const std::string& scan_id) const {
    if (IsRescan(scan_id))
        return scan2references.at(scan_id);
    else
        return "";
}

const bool Data::IsRescan(const std::string& scan_id) const {
    return (scan2references.find(scan_id) != scan2references.end());
}

const bool Data::IsReference(const std::string& scan_id) const {
    return !IsRescan(scan_id);
}

const Eigen::Matrix4f Data::GetRescanTransform(const std::string& scan_id) const {
    if (rescans.find(scan_id) != rescans.end())
        return rescans.at(scan_id).rescan2reference;
    else
        return Eigen::Matrix4f::Identity();
}

const Eigen::Matrix4f Data::GetRigidTransform(const std::string& scan_id, const int& instance) const {
    if (rescans.find(scan_id) != rescans.end()) {
        const MapIntMatrix4fAligned& rigids = rescans.at(scan_id).rigid_transforms;
        if (rigids.find(instance) != rigids.end())
            return rigids.at(instance);
    }
    return Eigen::Matrix4f::Identity();
}

const std::map<std::string, std::string>& Data::GetScan2References() const {
    return scan2references;
}
