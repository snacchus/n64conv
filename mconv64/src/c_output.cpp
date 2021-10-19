#include "c_output.hpp"

#include <cstdio>

void output_c(const mesh &mesh, const std::string &filename)
{
    std::FILE *output = std::fopen(filename.c_str(), "w");
    if (output == nullptr) {
        exit(5);
    }

    auto vertices_str = mesh.name + "_vertices";
    auto cmd_list_str = mesh.name + "_cmd_list";

    auto vertices_name = vertices_str.c_str();
    auto cmd_list_name = cmd_list_str.c_str();

    std::fprintf(output, "#ifndef %s_H\n", mesh.name.c_str());
    std::fprintf(output, "#define %s_H\n\n", mesh.name.c_str());

    std::fprintf(output, "#include <libdragon.h>\n\n");

    std::fprintf(output, "const ugfx_vertex_t %s[] = {\n", vertices_name);

    for (auto &vertex : mesh.vertices)
    {
        std::fprintf(output, "    {");
        std::fprintf(output, " .x = %i,", vertex.x);
        std::fprintf(output, " .y = %i,", vertex.y);
        std::fprintf(output, " .z = %i,", vertex.z);
        std::fprintf(output, " .s = %i,", vertex.s);
        std::fprintf(output, " .t = %i,", vertex.t);
        if (vertex._unused > 0)
        {
            std::fprintf(output, " .attr.color.r = %u,", uint32_t(vertex.c.r));
            std::fprintf(output, " .attr.color.g = %u,", uint32_t(vertex.c.g));
            std::fprintf(output, " .attr.color.b = %u,", uint32_t(vertex.c.b));
            std::fprintf(output, " .attr.color.a = %u,", uint32_t(vertex.c.a));
        }
        else
        {
            std::fprintf(output, " .attr.normal.x = %i,", int32_t(vertex.n.x));
            std::fprintf(output, " .attr.normal.y = %i,", int32_t(vertex.n.y));
            std::fprintf(output, " .attr.normal.z = %i,", int32_t(vertex.n.z));
            std::fprintf(output, " .attr.normal.a = %u,", uint32_t(vertex.n.a));
        }
        std::fprintf(output, " },\n");
    }

    std::fprintf(output, "};\n\n");

    std::fprintf(output, "const uint32_t %s_length = sizeof(%s) / sizeof(ugfx_vertex_t);\n\n", vertices_name, vertices_name);

    std::fprintf(output, "ugfx_command_t %s[] = {\n", cmd_list_name);

    for (auto &batch : mesh.batches)
    {
        uint32_t v0 = 0;
        for (auto &block : batch.blocks)
        {
            std::fprintf(output, "    ugfx_load_vertices(1, %u * sizeof(ugfx_vertex_t), %u, %u),\n", block.offset, v0, block.n);

            v0 += block.n;
        }

        for (auto &triangle : batch.triangles)
        {
            std::fprintf(output, "    ugfx_draw_triangle(%u, %u, %u),\n", triangle.indices[0], triangle.indices[1], triangle.indices[2]);
        }
    }

    std::fprintf(output, "    ugfx_finalize()\n");
    std::fprintf(output, "};\n\n");

    std::fprintf(output, "const uint32_t %s_length = sizeof(%s) / sizeof(ugfx_command_t);\n\n", cmd_list_name, cmd_list_name);

    std::fprintf(output, "#endif\n");
}

