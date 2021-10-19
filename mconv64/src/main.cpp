#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "binary_output.hpp"
#include "c_output.hpp"
#include "cxxopts.hpp"
#include "mesh.hpp"
#include "util.hpp"

int main(int argc, char **argv)
{
    cxxopts::Options options("mconv64", "Converts meshes into a N64-compatible format");
    options.add_options()
        ("i,input", "Input file name", cxxopts::value<std::string>())
        ("o,output", "Output file name", cxxopts::value<std::string>())
        ("c,code", "Output C header instead of binary")
        ("m,mesh-name", "Append mesh name to output file name")
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
    auto append_mesh_name = result.count("mesh-name");

    auto import_flags = aiProcess_Triangulate |
                        aiProcess_SortByPType |
                        aiProcess_ImproveCacheLocality |
                        aiProcess_FindDegenerates |
                        aiProcess_GenUVCoords |
                        aiProcess_FindInvalidData |
                        aiProcess_GenSmoothNormals |
                        aiProcess_JoinIdenticalVertices |
                        aiProcess_FlipUVs;
    
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    auto scene = importer.ReadFile(input_file_name, import_flags);

    if (scene == nullptr) 
    {
        std::cout << importer.GetErrorString() << std::endl;
        exit(3);
    }

    if (!scene->HasMeshes()) {
        std::cout << "No meshes in scene" << std::endl;
        exit(4);
    }

    for (auto i = 0; i < scene->mNumMeshes; ++i) {
        const auto processed_mesh = mesh::from_assimp_mesh(scene->mMeshes[i]);

        auto filename = output_file_name;
        if (append_mesh_name)
        {
            filename += processed_mesh.name;
        }

        if (result.count("code"))
        {
            output_c(processed_mesh, filename);
        }
        else 
        {
            output_binary(processed_mesh, filename);
        }
    }

    return 0;
}
