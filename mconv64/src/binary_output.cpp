#include "binary_output.hpp"

#include <fstream>

#define MC64_VERSION 1

#define UGFX_OP_FINALIZE               0x80ULL
#define UGFX_OP_LOAD_VERTICES          0x81ULL
#define UGFX_OP_DRAW_TRIANGLE          0x8DULL

#define __ugfx_mask_shift(x, mask, shift) (((uint64_t)(x) & mask) << shift)
#define __ugfx_opcode(opcode) __ugfx_mask_shift((opcode), 0xFF, 56)

#define ugfx_finalize() __ugfx_opcode(UGFX_OP_FINALIZE)

#define ugfx_load_vertices(slot, vertices, index, count) ( \
    __ugfx_opcode(UGFX_OP_LOAD_VERTICES) | \
    __ugfx_mask_shift((count), 0x3F, 44) | \
    __ugfx_mask_shift((index), 0x3F, 36) | \
    __ugfx_mask_shift((slot), 0xF, 28) | \
    __ugfx_mask_shift((uintptr_t)(vertices), 0x1FFFFFF, 0))

#define ugfx_draw_triangle(v0, v1, v2) ( \
    __ugfx_opcode(UGFX_OP_DRAW_TRIANGLE) | \
    __ugfx_mask_shift((v0), 0x3F, 49) | \
    __ugfx_mask_shift((v1), 0x3F, 43) | \
    __ugfx_mask_shift((v2), 0x3F, 37))

template<typename T>
void write_be(std::ostream &stream, const T &v)
{
    const auto size = sizeof(T);
    uint8_t buffer[size];

    for (auto i = 0; i < size; ++i)
    {
        buffer[i] = (v >> (8 * (size - i - 1))) & 0xFF;
    }

    stream.write((char*)buffer, size);
}

template<>
void write_be<vertex>(std::ostream &stream, const vertex &vertex)
{
    write_be(stream, vertex.x);
    write_be(stream, vertex.y);
    write_be(stream, vertex.z);
    write_be(stream, uint16_t(0));
    write_be(stream, vertex.s);
    write_be(stream, vertex.t);
    write_be(stream, vertex.c.r);
    write_be(stream, vertex.c.g);
    write_be(stream, vertex.c.b);
    write_be(stream, vertex.c.a);
}

template<typename T>
void write_be_vector(std::ostream &stream, const std::vector<T> &vector)
{
    for (auto &item : vector)
    {
        write_be(stream, item);
    }
}

std::vector<uint64_t> make_cmd_list(const mesh &mesh)
{
    std::vector<uint64_t> cmd_list;

    for (auto &batch : mesh.batches)
    {
        auto v0 = 0;
        for (auto &block : batch.blocks)
        {
            cmd_list.push_back(ugfx_load_vertices(1, block.offset * sizeof(vertex), v0, block.n));
            v0 += block.n;
        }

        for (auto &triangle : batch.triangles)
        {
            cmd_list.push_back(ugfx_draw_triangle(triangle.indices[0], triangle.indices[1], triangle.indices[2]));
        }
    }

    cmd_list.push_back(ugfx_finalize());

    return cmd_list;
}

void output_binary(const mesh &mesh, const std::string &filename)
{
    std::ofstream output(filename, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!output) {
        exit(5);
    }
    output.imbue(std::locale::classic());

    const auto cmd_list = make_cmd_list(mesh);

    const uint32_t vertices_size = sizeof(vertex) * mesh.vertices.size();
    const uint32_t cmd_list_size = sizeof(uint64_t) * cmd_list.size();

    output.write("MC64", 4);
    write_be(output, uint32_t(MC64_VERSION));
    write_be(output, vertices_size);
    write_be(output, cmd_list_size);

    write_be_vector(output, mesh.vertices);
    write_be_vector(output, cmd_list);
}
