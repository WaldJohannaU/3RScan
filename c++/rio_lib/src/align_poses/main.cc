#include <rio_lib/rio_config.h>
#include <rio_lib/rio.h>
#include <sys/stat.h>
#include <fstream>

int main(int argc, char **argv){
    if (argc < 3)
        return 0;
    const std::string data_path{argv[1]};
    const std::string scan_id{argv[2]};
    const std::string output_folder = (argc == 4) ? argv[3] : "sequence";
    RIO::RIOConfig config(data_path);
    RIO::RIO rio(config);

    size_t frame_id = 0;
    bool state;
    bool mm = false;
    while (true) {
        Eigen::Matrix4f pose_normalized;
        state = rio.GetCameraPose(pose_normalized, scan_id, frame_id, true, mm);
        if (!state) break;
        std::stringstream filename;
        const std::string pose_subfix = ".align.pose.txt";
        filename << data_path << "/" << scan_id << "/" << output_folder << "/"
                 << "frame-" << std::setfill('0') << std::setw(6) << frame_id << pose_subfix;
        std::fstream f(filename.str(), std::ios::out);
        if (!f) throw std::system_error(errno, std::system_category(), "failed to open "+filename.str());
        f << pose_normalized;
        f.close();
        frame_id++;
    }
    std::cout << "saved " << frame_id << " poses, ";
    std::cout << "in " << data_path << "/" << scan_id << "/" << output_folder << std::endl;
    return 0;
}