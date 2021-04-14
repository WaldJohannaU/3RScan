#include "rio_lib/rio.h"

#include <fstream>
// #include <stdlib.h>

#include "third_party/tinyply.h"

namespace RIO {

RIO::RIO(const RIOConfig& config): config_(config),
                                   data_config_(config_.data_path),
                                   json_data_(data_config_.GetJson()),
                                   sequence_(config_.data_path, json_data_) {
    const std::string& object_json = data_config_.GetObjectJson();
    LoadObjects(object_json);
}

const std::string RIO::GetReference(const std::string& scan_id) const {
    return json_data_.GetReference(scan_id);
}

const bool RIO::IsRescan(const std::string& scan_id) const {
    return json_data_.IsRescan(scan_id);
}

const bool RIO::IsReference(const std::string& scan_id) const {
    return json_data_.IsReference(scan_id);
}

const bool RIO::ReSavePLYASCII(const std::string& scan_id) const {
    RIOPlyData ply_file;
    const uint32_t vertices = ply_file.load(data_config_.GetInstance(scan_id));
    ply_file.save(data_config_.GetInstance(scan_id, ".ascii"), true);
    return (vertices > 0);
}

const bool RIO::Transform2Reference(const std::string& scan_id) const {
    if (json_data_.IsReference(scan_id)) {
        std::cout << "Warning: scan ID is a reference!" << std::endl;
        return false;
    }
    // Transform *.ply (labels file)
    const bool ply_success = TransformPly2Reference(scan_id,
                                                    data_config_.GetInstance(scan_id),
                                                    data_config_.GetInstance(scan_id, ".align"));
    // Transform *.obj file (3D model)
    const bool obj_success = TransformObj2Reference(scan_id,
                                                    data_config_.GetMesh(scan_id),
                                                    data_config_.GetMesh(scan_id, ".align"));
    return ply_success && obj_success;
}

bool RIO::TransformPly2Reference(const std::string& scan_id,
                                 const std::string& input, const std::string& output) const {
    std::ifstream ss(input, std::ios::binary);
    RIOPlyData ply_file;
    const uint32_t vertices = ply_file.load(input);
    const Eigen::Matrix4f& matrix = json_data_.GetRescanTransform(scan_id);
    for (int i = 0; i < vertices; i++) {
        const Eigen::Vector4f vertex(ply_file.vertices[3*i], ply_file.vertices[3*i+1], ply_file.vertices[3*i+2], 1);
        const Eigen::Vector4f vertex_transformed = matrix * vertex;
        ply_file.vertices[3*i] = vertex_transformed(0);
        ply_file.vertices[3*i+1] = vertex_transformed(1);
        ply_file.vertices[3*i+2] = vertex_transformed(2);
    }
    ply_file.save(output, true);
    return vertices > 0;
}

bool RIO::TransformObj2Reference(const std::string& scan_id,
                                 const std::string& input,
                                 const std::string& output) const {
    std::ifstream obj_file_in(input);
    std::ofstream obj_file_out(output);

    std::string line;
    std::string tag;
    std::vector<std::string> faces;
    const Eigen::Matrix4f& matrix = json_data_.GetRescanTransform(scan_id);
    Eigen::Vector4f vertex(0,0,0,1);
    if (obj_file_in.is_open()) {
        if (obj_file_out.is_open()) {
            while (getline(obj_file_in, line)) {
                std::string type = line.substr(0, line.find(" "));
                if (type == "v") {
                    std::istringstream iss(line);
                    iss >> tag >> vertex(0) >> vertex(1) >> vertex(2);
                    Eigen::Vector4f vertex_transformed = matrix * vertex;
                    obj_file_out << tag << " "
                                 << vertex_transformed(0) << " "
                                 << vertex_transformed(1) << " "
                                 << vertex_transformed(2) << std::endl;
                } else obj_file_out << line << std::endl;
            }
            obj_file_out.close();
            obj_file_in.close();
            std::cout << "saved file: " << output << std::endl;
            return true;
        }
    }
    return false;
}

const bool RIO::RemapLabelsPly(const std::string& scan_id) const {
    if (scans.find(scan_id) != scans.end()) {
        const Scan& scan = scans.at(scan_id);
        RIOPlyData ply_file;
        const uint32_t vertices = ply_file.load(data_config_.GetInstance(scan_id));
        for (int i = 0; i < vertices; i++) {
            const int instance_id = ply_file.object_ids[i];
            int global_id = 0;
            if (scan.instance2global.find(instance_id) != scan.instance2global.end()) {
                global_id = scan.instance2global.at(instance_id);
            }
            ply_file.global_ids.push_back(global_id);
            // also remap colors
            ply_file.colors[3*i] = globalId2color[global_id](0);
            ply_file.colors[3*i+1] = globalId2color[global_id](1);
            ply_file.colors[3*i+2] = globalId2color[global_id](2);
        }
        std::cout << "save " << data_config_.GetInstance(scan_id, ".global") << std::endl;
        ply_file.save(data_config_.GetInstance(scan_id, ".global"), true);
        return vertices > 0;
    }
    return false;
}

const Eigen::Matrix4f RIO::GetCameraPose(const std::string& scan_id, 
                                         const int frame_id,
                                         const bool normalize2reference,
                                         const bool mm) const {
    bool valid_pose = false;
    return sequence_.GetPose(scan_id, frame_id, normalize2reference, mm, valid_pose);
}

const bool RIO::GetCameraPose(Eigen::Matrix4f& pose,
                              const std::string& scan_id, 
                              const int frame_id,
                              const bool normalize2reference,
                              const bool mm) const {
    bool valid_pose = false;    
    pose = sequence_.GetPose(scan_id, frame_id, normalize2reference, mm, valid_pose);
    return valid_pose;
}

const bool RIO::Backproject(const std::string& scan_id, const int frame_id,
                            const bool normalized2reference) const {
    return sequence_.Backproject(scan_id, frame_id, normalized2reference);
}

void RIO::InitGlobalId2Color(const int size) {
    // let's fix the seed to make sure we always get the same colors.
    std::srand(0);
    // instance with index 0 is invalid.
    globalId2color.push_back(Eigen::Vector3i(0,0,0));
    for (int i = 0; i < size; i++) {
    	const int b = std::rand() % (color_range_from - color_range_to) + color_range_from;
        const int g = std::rand() % (color_range_from - color_range_to) + color_range_from;
        const int r = std::rand() % (color_range_from - color_range_to) + color_range_from;
        globalId2color.push_back(Eigen::Vector3i(r,g,b));
    }
}

const bool RIO::LoadObjects(const std::string& objects) {
    InitGlobalId2Color(kglobalId_size);
    std::ifstream is(objects);
    std::string dataset((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    std::string err;
    const auto json = json11::Json::parse(dataset, err);
    if (err != "")
        return false;
    // Iterates through all the scans.
    for (auto &scan_json: json["scans"].array_items()) {
        const auto& objects = scan_json["objects"].array_items();
        Scan scan;
        for (const auto& obj: objects) {
            const int id = std::stoi(obj["id"].string_value());
            const int global_id = std::stoi(obj["global_id"].string_value());
            scan.instance2labels[id] = obj["label"].string_value();
            scan.instance2global[id] = global_id;
        }
        const std::string& scan_id = scan_json["scan"].string_value();
        scans[scan_id] = scan;
    }
    return !scans.empty();
}

void RIO::PrintSemanticLabels(const std::string& scan_id) const {
    // Writes semantic labels to the console.
    if (scans.find(scan_id) != scans.end()) {
        const auto& scan = scans.at(scan_id);
        for (const auto instance: scan.instance2labels) {
            std::cout << instance.first << ": "
                      << scan.instance2labels.at(instance.first)
                      << " (" << scan.instance2global.at(instance.first) << ")" << std::endl;
        }
    }
}

const bool RIO::TransformInstance(const std::string& scan_id, const int& instance) const {
    RIOPlyData ply_file;
    ply_file.load(data_config_.GetInstance(scan_id));
    
    std::vector<int> vertices_to_keep;
    std::vector<int> faces_to_keep;
    const int faces_size = static_cast<int>(ply_file.faces.size() / 3);
    for (int i = 0; i < faces_size; i++) {
        // if any of the vertices of that face belong to the label we keep the face.
        if ((ply_file.object_ids[ply_file.faces[3*i]] == instance) ||
            (ply_file.object_ids[ply_file.faces[3*i+1]] == instance) ||
            (ply_file.object_ids[ply_file.faces[3*i+2]] == instance)) {
            faces_to_keep.push_back(i);
        }
    }
    // let's get the matrix that transforms the rigid objects.
    Eigen::Matrix4f matrix = json_data_.GetRigidTransform(scan_id, instance);
    std::cout << matrix << std::endl;
    std::ifstream obj_file_in(data_config_.GetMesh(scan_id));
    std::ofstream obj_file_out(data_config_.GetMesh(scan_id, ".align.instance." + std::to_string(instance)));
    Eigen::Vector4f vertex(0,0,0,1);
    std::string tag;
    if (!faces_to_keep.empty()) {
        std::string line;
        std::vector<std::string> faces;
        if (obj_file_in.is_open()) {
            if (obj_file_out.is_open()) {
                // TODO this can be done easier
                while (getline(obj_file_in, line)) {
                    std::string type = line.substr(0, line.find(" "));
                    if (type == "f")
                        faces.push_back(line);
                    else if (type == "v") {
                        // TODO We keep all the vertices
                        // (not ideal but that helps to avoid re-mapping the faces)
                        std::istringstream iss(line);
                        iss >> tag >> vertex(0) >> vertex(1) >> vertex(2);
                        Eigen::Vector4f vertex_transformed = matrix * vertex;
                        obj_file_out << tag << " "
                                     << vertex_transformed(0) << " "
                                     << vertex_transformed(1) << " "
                                     << vertex_transformed(2) << std::endl;
                    }
                    else obj_file_out << line << std::endl;
                }
                // Saves the faces we want to keep.
                for (const int face_id: faces_to_keep) {
                    obj_file_out << faces[face_id] << std::endl;
                }
                obj_file_out.close();
                obj_file_in.close();
            }
        }
        return true;
    }
    return false;
}

}  // namespace rio
