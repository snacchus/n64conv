#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "cxxopts.hpp"

int main(int argc, char** argv) {
    cxxopts::Options options("tconv64", "Converts textures into a N64-compatible format");
    options.add_options()
        ("i,input", "Input file name", cxxopts::value<std::string>())
        ("o,output", "Output file name", cxxopts::value<std::string>())
        ("h,help", "Print help text");

    options.parse_positional({"input"});
    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (!result.count("input"))
    {
        std::cout << "No input file specified." << std::endl;
        exit(1);
    }

    if (!result.count("output"))
    {
        std::cout << "No output file specified." << std::endl;
        exit(2);
    }

    auto input_file_name = result["input"].as<std::string>();
    auto output_file_name = result["output"].as<std::string>();

    int w, h, comp, req_comp = 4;
    auto data = stbi_load(input_file_name.c_str(), &w, &h, &comp, req_comp);

    if (data == nullptr)
    {
        std::cout << "Failed reading image file: " << stbi_failure_reason() << std::endl;
        exit(3);
    }

    auto size = req_comp * w * h;

    std::ofstream output(output_file_name, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!output) 
    {
        exit(4);
    }
    output.imbue(std::locale::classic());

    output.write((const char*)data, size);

    stbi_image_free(data);
    return 0;
}
