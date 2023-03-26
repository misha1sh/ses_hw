#include "common.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdio>

using FileContent = std::vector<std::pair<std::string, uint32_t>>;
std::optional<FileContent> ReadFile(const std::string file_name) {
    FileContent res;

    std::ifstream file{file_name};
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << file_name << std::endl;
        return std::nullopt;
    }

    std::string s;
    while (std::getline(file, s)) {
        const auto pos = s.find(" : ");
        if (pos == std::string::npos || s.size() <= pos + 3) {
            std::cerr << file_name  << ": Illegally formated file. Line: " << s << std::endl;
            return std::nullopt;
        }

        const std::string_view part1 = s.substr(0, pos);
        const std::string_view part2 = s.substr(pos + 3);
        const auto value_opt = ParseUint32OrError(part2, part1);
        if (!value_opt) {
            std::cerr << "Error while reading " << file_name << ". Line " << s << std::endl;
            return std::nullopt;
        }
        res.emplace_back(part1, *value_opt);
    }

    return res;
}

int main(int argc, char** argv) {
    std::filesystem::path executable_path{argv[0]};
    std::filesystem::path dir_path{executable_path.parent_path()};
    ReadFile(dir_path / "op1.txt");
    ReadFile(dir_path / "op2.txt");
    ReadFile(dir_path / "queue.txt");
}