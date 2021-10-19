#include "mesh.hpp"

#include <assimp/mesh.h>

#include "util.hpp"

uint32_t vertex_block::get_new_size(uint32_t index) const 
{
    auto up = int32_t(index) - int32_t(offset) + 1;
    auto down = int32_t(n) - int32_t(up);
    return uint32_t(std::max({up, down, int32_t(n)}));
}

void vertex_block::insert(uint32_t index)
{
    if (index < offset)
    {
        n += offset - index;
        offset = index;
    }
    else if (index >= offset + n)
    {
        n = index - offset + 1;
    }
}

uint32_t get_vertex_count(const std::vector<vertex_block> &blocks)
{
    uint32_t result = 0;

    for (auto &block : blocks)
    {
        result += block.n;
    }

    return result;
}

int32_t find_best_fit(const std::vector<vertex_block> &blocks, uint32_t index, uint32_t reserve)
{
    int32_t block_index = -1;
    auto new_size = std::numeric_limits<uint32_t>::max();

    for (auto i = 0; i < blocks.size(); ++i)
    {
        auto tmp_new_size = blocks[i].get_new_size(index);
        if ((tmp_new_size < new_size) && (tmp_new_size + reserve <= MAX_BATCH_SIZE))
        {
            new_size = tmp_new_size;
            block_index = i;
        }
    }

    return block_index;
}

uint32_t triangle_batch::transform_index(uint32_t index, uint32_t block_index) const
{
    auto block_offset = 0;

    for (auto i = 0; i < block_index; ++i)
    {
        block_offset += blocks[i].n;
    }

    return block_offset + index - blocks[block_index].offset;
}

bool triangle_batch::try_add_triangle(const uint32_t *indices, std::array<uint32_t, 3> &block_indices)
{
    auto tmp_blocks = blocks;

    for (auto i = 0; i < 3; ++i)
    {
        auto index = indices[i];
        auto block_index = find_best_fit(tmp_blocks, index, 2 - i);

        if (block_index < 0)
        {
            vertex_block new_block;
            new_block.offset = index;
            new_block.n = 1;
            block_index = tmp_blocks.size();
            tmp_blocks.push_back(new_block);
        }
        else
        {
            tmp_blocks[block_index].insert(index);
        }

        block_indices[i] = block_index;
    }

    if (get_vertex_count(tmp_blocks) <= MAX_BATCH_SIZE)
    {
        blocks = tmp_blocks;
        return true;
    }
    else 
    {
        return false;
    }
}

mesh mesh::from_assimp_mesh(const aiMesh *assimp_mesh)
{
    mesh output_mesh;

    // Convert vertices
    for (auto i = 0; i < assimp_mesh->mNumVertices; ++i)
    {
        vertex v;

        auto pos = assimp_mesh->mVertices[i];

        v.x = flt_to_s10_5(pos.x);
        v.y = flt_to_s10_5(pos.y);
        v.z = flt_to_s10_5(pos.z);

        if (assimp_mesh->HasTextureCoords(0))
        {
            auto tex = assimp_mesh->mTextureCoords[0][i];

            v.s = flt_to_s10_5(tex.x * 32.f) << 1;
            v.t = flt_to_s10_5(tex.y * 32.f) << 1;
        }

        if (assimp_mesh->HasVertexColors(0))
        {
            auto color = assimp_mesh->mColors[0][i];

            v.c.r = normalize_flt_u8(color.r);
            v.c.g = normalize_flt_u8(color.g);
            v.c.b = normalize_flt_u8(color.b);
            v.c.a = normalize_flt_u8(color.a);

            v._unused = 1;
        }
        else if (assimp_mesh->HasNormals())
        {
            auto normal = assimp_mesh->mNormals[i];

            v.n.x = normalize_flt_i8(normal.x);
            v.n.y = normalize_flt_i8(normal.y);
            v.n.z = normalize_flt_i8(normal.z);

            v._unused = 0;
        }

        output_mesh.vertices.push_back(v);
    }

    // Form triangle batches
    std::vector<const aiFace*> input_triangles;
    for (auto i = 0; i < assimp_mesh->mNumFaces; ++i)
    {
        auto &face = assimp_mesh->mFaces[i];
        if (face.mNumIndices == 3)
        {
            input_triangles.push_back(&face);
        }
    }

    std::size_t triangle_count = 0;

    while (triangle_count < input_triangles.size())
    {
        triangle_batch batch;
        std::size_t triangle_index = triangle_count;
        std::vector<std::array<uint32_t, 3>> block_indices_per_triangle;

        for (; triangle_index < input_triangles.size(); ++triangle_index)
        {
            std::array<uint32_t, 3> block_indices;
            auto triangle = input_triangles[triangle_index];
            if (!batch.try_add_triangle(triangle->mIndices, block_indices))
            {
                break;
            }
            block_indices_per_triangle.push_back(block_indices);
        }

        for (auto i = triangle_count; i < triangle_index; ++i)
        {
            triangle new_triangle;
            auto triangle = input_triangles[i];
            auto &block_indices = block_indices_per_triangle[i - triangle_count];
            for (auto j = 0; j < 3; ++j)
            {
                auto index = triangle->mIndices[j];
                auto block_index = block_indices[j];
                new_triangle.indices[j] = batch.transform_index(index, block_index);
            }
            batch.triangles.push_back(new_triangle);
        }

        output_mesh.batches.push_back(batch);
        triangle_count += batch.triangles.size();
    }

    // Set name
    auto name = std::string(assimp_mesh->mName.C_Str());
    std::transform(name.begin(), name.end(), name.begin(), [](auto c){ return std::tolower(c); });
    output_mesh.name = name;

    return output_mesh;
}
