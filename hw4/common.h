#include <cstdlib>
#include <optional>
#include <string>
#include <iostream>

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

std::optional<size_t> GetTableLimit() {
    static const std::string kTableLimit = "TABLE_LIMIT";
    if (const char* result = std::getenv(kTableLimit.c_str())) {
        return ParseUint32OrError(result, kTableLimit);
    }

    std::cerr << "Env variable " << kTableLimit << "' not found" << std::endl;
    return std::nullopt;
}