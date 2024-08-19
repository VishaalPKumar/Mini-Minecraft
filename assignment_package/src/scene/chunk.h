#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"

const static float BLK_UV = 1.f / 16.f;

//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, SAND, BEDROCK, LAVA
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

static bool isTransparent(BlockType b) {
    return b == WATER || b == LAVA;
}

static bool isAnimatable(BlockType b) {
    return b == WATER || b == LAVA;
}

static bool isSolidBlock(BlockType cellType) {
    return cellType != EMPTY && cellType != WATER && cellType != LAVA;
}

static bool isWaterLavaBlock(BlockType cellType) {
    return cellType == WATER || cellType == LAVA;
}

static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockFaceUVs {


    {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash>{  {XPOS, glm::vec2(3.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(3.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(8.f * BLK_UV, 13.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(3.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(3.f * BLK_UV, 15.f * BLK_UV)}}},
    {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)}}},
    {STONE, std::unordered_map<Direction, glm::vec2, EnumHash>{  {XPOS, glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)}}},
    {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)}}},
    {SAND, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(2.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(2.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(2.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(2.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(2.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(2.f * BLK_UV, 14.f * BLK_UV)}}},
    {BEDROCK, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)}}},
    {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>{   {XPOS, glm::vec2(14.f * BLK_UV, 1.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(14.f * BLK_UV, 1.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(14.f * BLK_UV, 1.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(14.f * BLK_UV, 1.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(14.f * BLK_UV, 1.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(14.f * BLK_UV, 1.f * BLK_UV)}}},
    {WATER, std::unordered_map<Direction, glm::vec2, EnumHash>{  {XPOS, glm::vec2(14.f * BLK_UV, 3.f * BLK_UV)},
                                                                 {XNEG, glm::vec2(14.f * BLK_UV, 3.f * BLK_UV)},
                                                                 {YPOS, glm::vec2(14.f * BLK_UV, 3.f * BLK_UV)},
                                                                 {YNEG, glm::vec2(14.f * BLK_UV, 3.f * BLK_UV)},
                                                                 {ZPOS, glm::vec2(14.f * BLK_UV, 3.f * BLK_UV)},
                                                                 {ZNEG, glm::vec2(14.f * BLK_UV, 3.f * BLK_UV)}}},
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

static std::unordered_map<Direction, glm::ivec3, EnumHash> directionToNormal = {
    {XPOS, glm::ivec3(1,0,0)},
    {XNEG, glm::ivec3(-1,0,0)},
    {YPOS, glm::ivec3(0,1,0)},
    {YNEG, glm::ivec3(0,-1,0)},
    {ZPOS, glm::ivec3(0,0,1)},
    {ZNEG, glm::ivec3(0,0,-1)},
};

struct VertexData {
    glm::vec4 pos;
    glm::vec4 nor;
    glm::vec2 uv;
    float animatable = 0.f;

    VertexData(const glm::vec4& position, const glm::vec2& uv)
        : pos(position), nor(glm::vec4(0,0,0,0)), uv(uv), animatable(0.f) {}
};

struct AdjacentFace {
    Direction d;
    glm::ivec3 direction;
    std::array<VertexData, 4> vertices;
    AdjacentFace(Direction d, std::array<VertexData, 4> vertices)
        : d(d), vertices(vertices) {
        // direction = directionToNormal.at(d);
        switch (d) {
        case XPOS:
            direction = glm::ivec3(1,0,0);
            break;
        case XNEG:
            direction = glm::ivec3(-1,0,0);
            break;
        case YPOS:
            direction = glm::ivec3(0,1,0);
            break;
        case YNEG:
            direction = glm::ivec3(0,-1,0);
            break;
        case ZPOS:
            direction = glm::ivec3(0,0,1);
            break;
        case ZNEG:
            direction = glm::ivec3(0,0,-1);
            break;
        }
    }
};

class Chunk;

struct ChunkVBOData {
    Chunk* mp_chunk;
    std::vector<VertexData> m_VBODataOpaque, m_VBODataTransparent;
    std::vector<GLuint> m_idxDataOpaque, m_idxDataTransparent;

    ChunkVBOData(Chunk* c) : mp_chunk(c), m_VBODataOpaque{}, m_VBODataTransparent{},
        m_idxDataOpaque{}, m_idxDataTransparent{}
    {}
};

const static std::array<AdjacentFace, 6> adjacentBlocks {
    // Positive x face
    AdjacentFace(XPOS, {VertexData(glm::vec4(1,0,0,1), glm::vec2(BLK_UV,0)),
                        VertexData(glm::vec4(1,1,0,1), glm::vec2(BLK_UV,BLK_UV)),
                        VertexData(glm::vec4(1,1,1,1), glm::vec2(0,BLK_UV)),
                        VertexData(glm::vec4(1,0,1,1), glm::vec2(0,0))}),

    // Negative x face
    AdjacentFace(XNEG, {VertexData(glm::vec4(0,0,1,1), glm::vec2(BLK_UV,0)),
                        VertexData(glm::vec4(0,1,1,1), glm::vec2(BLK_UV,BLK_UV)),
                        VertexData(glm::vec4(0,1,0,1), glm::vec2(0,BLK_UV)),
                        VertexData(glm::vec4(0,0,0,1), glm::vec2(0,0))}),

    // Positive y face
    AdjacentFace(YPOS, {VertexData(glm::vec4(1,1,1,1), glm::vec2(BLK_UV,0)),
                        VertexData(glm::vec4(1,1,0,1), glm::vec2(BLK_UV,BLK_UV)),
                        VertexData(glm::vec4(0,1,0,1), glm::vec2(0,BLK_UV)),
                        VertexData(glm::vec4(0,1,1,1), glm::vec2(0,0))}),

    // Negative y face
    AdjacentFace(YNEG, {VertexData(glm::vec4(1,0,1,1), glm::vec2(BLK_UV,0)),
                        VertexData(glm::vec4(1,0,0,1), glm::vec2(BLK_UV,BLK_UV)),
                        VertexData(glm::vec4(0,0,0,1), glm::vec2(0,BLK_UV)),
                        VertexData(glm::vec4(0,0,1,1), glm::vec2(0,0))}),

    // Positive z face
    AdjacentFace(ZPOS, {VertexData(glm::vec4(1,0,1,1), glm::vec2(BLK_UV,0)),
                        VertexData(glm::vec4(1,1,1,1), glm::vec2(BLK_UV,BLK_UV)),
                        VertexData(glm::vec4(0,1,1,1), glm::vec2(0,BLK_UV)),
                        VertexData(glm::vec4(0,0,1,1), glm::vec2(0,0))}),

    // Negative z face
    AdjacentFace(ZNEG, {VertexData(glm::vec4(0,0,0,1), glm::vec2(BLK_UV,0)),
                        VertexData(glm::vec4(0,1,0,1), glm::vec2(BLK_UV,BLK_UV)),
                        VertexData(glm::vec4(1,1,0,1), glm::vec2(0,BLK_UV)),
                        VertexData(glm::vec4(1,0,0,1), glm::vec2(0,0))}),
    };

class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // The coordinates of the chunk's lower-left corner in world space
    int minX, minZ;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    ChunkVBOData m_vboData;
    bool vboDataCreated;

public:
    Chunk(OpenGLContext* context, int x, int z);
    BlockType getLocalBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getLocalBlockAt(int x, int y, int z) const;
    void setLocalBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    GLenum drawMode() override;
    void createVBOdata() override;
    void destroyVBOdata() override;

    void create(std::vector<VertexData>&, std::vector<GLuint>&, std::vector<VertexData>&, std::vector<GLuint>&);
    // void destroyVBOdata() override; // potentially will override this later

    bool isVBODataCreated() const;
    void setVBODataCreated(bool);
    void createAndBufferVBOData();

    friend class VBOWorker;
};
