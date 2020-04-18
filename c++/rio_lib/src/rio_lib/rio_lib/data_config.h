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

struct DataConfig {
    const std::string base_path{""};
    const std::string json_file{"3RScan.json"};
    const std::string objects_json_file{"objects.json"};
    
    const std::string mesh{"mesh.refined"};
    const std::string texture{"mesh.refined_0.png"};
    const std::string instances{"labels.instances.annotated"};

    const std::string semseg{"semseg.json"};

    DataConfig(const std::string& base_path): base_path(base_path) { }

    const std::string GetObjectJson() const {
        return base_path + "/" + objects_json_file;
    }
    
    const std::string GetJson() const {
        return base_path + "/" + json_file;
    }
    
    const std::string GetTexture(const std::string& scan_id) const {
        return base_path + "/" + scan_id + "/" + texture;
    }
    
    const std::string GetMesh(const std::string& scan_id, const std::string suffix = "") const {
        return base_path + "/" + scan_id + "/" + mesh + suffix + ".obj";
    }
    
    const std::string GetInstance(const std::string& scan_id, const std::string suffix = "") const {
        return base_path + "/" + scan_id + "/" + instances + suffix + ".ply";
    }

    const std::string GetSemSeg(const std::string& scan_id) const {
        return base_path + "/" + scan_id + "/" + semseg;
    }

};
