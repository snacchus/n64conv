#ifndef MESH_HPP
#define MESH_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#define MAX_BATCH_SIZE 32

struct aiMesh;

struct vertex
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t _unused;
    int16_t s;
    int16_t t;
    union {
        uint32_t rgba32;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        } c;
        struct {
            int8_t x;
            int8_t y;
            int8_t z;
            uint8_t a;
        } n;
    };
};

struct triangle
{
    uint32_t indices[3];
};

struct vertex_block
{
    uint32_t offset;
    uint32_t n;

    uint32_t get_new_size(uint32_t index) const;
    void insert(uint32_t index);
};

struct triangle_batch
{
    std::vector<vertex_block> blocks;
    std::vector<triangle> triangles;

    uint32_t transform_index(uint32_t index, uint32_t block_index) const;
    bool try_add_triangle(const uint32_t *indices, std::array<uint32_t, 3> &block_indices);
};

struct mesh
{
    std::vector<triangle_batch> batches;
    std::vector<vertex> vertices;
    std::string name;

    static mesh from_assimp_mesh(const aiMesh *assimp_mesh);
};

#endif
