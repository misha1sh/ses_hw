#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>

struct ProgramArguments {
    std::filesystem::path input_directory;
    std::filesystem::path output_directory;
};

std::optional<ProgramArguments> ReadArguments(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Expected two arguments <input_directory> <output_directory>. Got " << argc - 1 << " arguments"
                  << std::endl;
        return std::nullopt;
    }

    return ProgramArguments {
            .input_directory = argv[1],
            .output_directory = argv[2],
    };
}

bool CreateDirectoryRecuresive(const std::filesystem::path & dirName)
{
    std::error_code err;
    if (!std::filesystem::create_directories(dirName, err))
    {
        if (std::filesystem::exists(dirName))
        {
            return true; 
        }
        std::cerr << "error during writing to " << dirName << std::endl;
        return false;
    }
    return true;
}

bool HasChanged(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
    if (!std::filesystem::exists(output_path)) {
        return true;
    }
    const auto input_last_change = std::filesystem::last_write_time(input_path);
    const auto backup_last_time = std::filesystem::last_write_time(output_path);
    return input_last_change > backup_last_time;
}

bool CreateGzip(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
    {
        std::ifstream inp(input_path);
        if (!inp.is_open()) {
            std::cerr << "Unable to open " << input_path << std::endl;
            return false;
        }
    }
    if (!CreateDirectoryRecuresive(output_path.parent_path())) {
        return false;
    }
    std::string command = std::string{"gzip -c "} + input_path.string() + " > " + output_path.string() ;// + " > /tmp/input_file.gzip";
    const auto ret_code = std::system(command.c_str());
    if (ret_code != 0) {
        std::cerr << "Failed to create gzip from " << input_path << " to " << output_path << " return_code: " << ret_code
                  << " command:" << command << std::endl;
        return false;
    }
    
    return true;
}


bool DoBackup(const ProgramArguments& args) {
     for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(args.input_directory)) {
        if (!dirEntry.is_regular_file()) {
            continue;
        }
        const auto& in_path = dirEntry.path();
        auto out_path = args.output_directory / std::filesystem::relative(in_path, args.input_directory);
        out_path += ".gz";

        if (HasChanged(in_path, out_path)) {
            std::cerr << "changed " << in_path << std::endl;
            if (!CreateGzip(in_path, out_path)) {
                return false;
            }
        }
     }
     return true;
}

int main(int argc, char** argv) {
    const auto args = ReadArguments(argc, argv);
    if (!args) {
        return -1;
    }
    const auto [input_directory, output_directory] = *args;

    if (!std::filesystem::is_directory(input_directory)) {
        std::cerr << "Incorrect a input directory" << std::endl;
        return -1;
    }
    if (!std::filesystem::is_directory(output_directory)) {
        std::cerr << "Incorrect a output directory" << std::endl;
        return -1;
    }
    
    if (!DoBackup(*args)) {
        return -1;
    }

    return 0;
}
