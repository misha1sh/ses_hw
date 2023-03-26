#include <cstdlib>
#include <optional>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

std::optional<uint32_t> ParseUint32OrError(const std::string_view s, const std::string_view name) {
    try {
        size_t pos;
        size_t res = std::stoi(std::string{s}, &pos);
        if (pos != s.size()) {
            std::cerr << "Invalid format for `" << name << "` : " << s << std::endl;
            return std::nullopt;
        }
        if (res < 0) {
            std::cerr << name << " cannot be negative. Got " << res << std::endl;
            return std::nullopt;
        }
        return res;
    } catch (std::invalid_argument) {
        std::cerr << "Invalid " << name << " : " << s << std::endl;
    } catch (std::out_of_range) {
        std::cerr << name << " is too big" << std::endl;
    }
    return std::nullopt;
}

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

using OperationsTable = std::unordered_map<std::string, uint32_t>;
OperationsTable FileContentToOperationsTable(const FileContent& file_content) {
    OperationsTable res;
    for (const auto& [op_name, op_time] : file_content) {
        res.emplace(op_name, op_time);
    }
    return res;
};


std::optional<size_t> GetTableLimit() {
    static const std::string kTableLimit = "TABLE_LIMIT";
    if (const char* result = std::getenv(kTableLimit.c_str())) {
        return ParseUint32OrError(result, kTableLimit);
    }

    std::cerr << "Env variable " << kTableLimit << "' not found" << std::endl;
    return std::nullopt;
}