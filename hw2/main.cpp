#include <algorithm>
#include <boost/iostreams/categories.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

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

bool HasChanged(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
    std::ifstream input{input_path, std::ios_base::in | std::ios_base::binary};

//    std::ifstream output{output_path, std::ios_base::in | std::ios_base::binary};
//    if (!output.is_open()) {
//        return true;
//    }
    if (!std::filesystem::is_regular_file(output_path)) {
        return true;
    }
    std::cerr << output_path << std::endl;

    boost::iostreams::filtering_istream outStream;
//    outStream.push(boost::iostreams::gzip_decompressor());
    outStream.push(boost::iostreams::file_source(output_path));
//    std::ifstream outStream{output_path, std::ios_base::in | std::ios_base::binary};

    std::cerr << outStream.size() << " " << outStream.is_complete() << " " << outStream.empty() << std::endl;

    constexpr size_t kBufferSize = 1024;
    std::array<char, kBufferSize> input_buffer{}, output_buffer{};
    while (input && outStream) {
        const size_t read_count1 = input.readsome(input_buffer.begin(), kBufferSize);
        const size_t read_count2 = outStream.readsome(output_buffer.begin(), kBufferSize);
        if (read_count1 != read_count2) {
            return true;
        }
        if (input_buffer != output_buffer) {
            return true;
        }
    }

    if (input.eof() != outStream.eof()) {
        return true;
    }

    return false;
}

bool CreateGzip(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
    std::ifstream input{input_path, std::ios_base::in | std::ios_base::binary};

    const auto parent_dir = output_path.parent_path();
    if (!std::filesystem::is_directory(parent_dir)) {
        std::filesystem::create_directories(parent_dir);
    }

    std::ofstream output{output_path, std::ios_base::out | std::ios_base::binary};
    boost::iostreams::filtering_ostream outStream;
    outStream.push(boost::iostreams::gzip_compressor());
    outStream.push(output);

    if (!input.is_open()) {
        std::cerr << "Unable to open " << input_path << std::endl;
        return false;
    }

    if (!output.is_open()) {
        std::cerr << "Unable to open " << output_path << std::endl;
        return false;
    }
    boost::iostreams::copy(input, outStream);
    return true;
}

void DoBackup(const ProgramArguments& args) {
    //for (const auto* )
}

int main(int argc, char** argv) {
    const auto args = ReadArguments(argc, argv);
    if (!args) {
        return -1;
    }
    const auto [input_directory, output_directory] = *args;

    if (!std::filesystem::is_directory(input_directory)) {
        std::cerr << "Not a input directory" << std::endl;
        return -1;
    }
    if (!std::filesystem::is_directory(output_directory)) {
        std::cerr << "Not a output directory" << std::endl;
        return -1;
    }

    for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(input_directory)) {
        if (!dirEntry.is_regular_file()) {
            continue;
        }
        const auto& in_path = dirEntry.path();
        auto out_path = output_directory / std::filesystem::relative(in_path, input_directory);
        out_path += ".gz";

        if (HasChanged(in_path, out_path)) {
            std::cerr << "changed " << in_path << std::endl;
            if (!CreateGzip(in_path, out_path)) {
                return -1;
            }
        }
    }
    return 0;
}
