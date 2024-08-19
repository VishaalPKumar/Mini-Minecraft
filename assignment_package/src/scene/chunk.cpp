#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext* context, int x, int z) : Drawable(context), m_blocks(), minX(x), minZ(z), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, m_vboData(ChunkVBOData(this)), vboDataCreated(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getLocalBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getLocalBlockAt(int x, int y, int z) const {
    return getLocalBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setLocalBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void Chunk::destroyVBOdata() {
    Drawable::destroyVBOdata();
    this->m_vboData.m_VBODataOpaque.clear();
    this->m_vboData.m_VBODataTransparent.clear();
    this->m_vboData.m_idxDataOpaque.clear();
    this->m_vboData.m_idxDataTransparent.clear();
}

void Chunk::createVBOdata() {
    std::vector<VertexData> data;
    std::vector<GLuint> idx;
    unsigned int numOpaqueFaces = 0;
    unsigned int numTransparentFaces = 0;

    for (int z = 0; z < 16; ++z) {
        for (int y = 0; y < 256; ++y) {
            for (int x = 0; x < 16; ++x) {
                BlockType currentBlock = this->getLocalBlockAt(x, y, z);
                bool currIsTransparent = isTransparent(currentBlock);

                if (currentBlock != EMPTY) {
                    for (const AdjacentFace &neighbor : adjacentBlocks) {
                        glm::ivec3 adj = glm::ivec3(x, y, z) + neighbor.direction;
                        int blockIndex = adj.x + 16 * adj.y + 16 * 256 * adj.z;
                        bool hasEmptyAdj = blockIndex >= 0 && blockIndex < this->m_blocks.size() &&
                                           (this->getLocalBlockAt(adj.x, adj.y, adj.z) == EMPTY ||
                                           (isTransparent(this->getLocalBlockAt(adj.x, adj.y, adj.z)) && !currIsTransparent));
                        bool isBoundaryFace = (x == 0 && neighbor.direction.x == -1) ||
                                              (x == 15 && neighbor.direction.x == 1) ||
                                              (y == 0 && neighbor.direction.y == -1) ||
                                              (y == 255 && neighbor.direction.y == 1) ||
                                              (z == 0 && neighbor.direction.z == -1) ||
                                              (z == 15 && neighbor.direction.z == 1);

                        if (isBoundaryFace) {
                            Chunk* neighbor_chunk = this->m_neighbors[neighbor.d];
                            if (neighbor_chunk != nullptr) {
                                int new_x = (adj.x + 16) % 16;
                                int new_y = (adj.y + 256) % 256;
                                int new_z = (adj.z + 16) % 16;

                                BlockType neighbor_block = neighbor_chunk->getLocalBlockAt(new_x, new_y, new_z);
                                if (neighbor_block == EMPTY || (isTransparent(neighbor_block) && !currIsTransparent)) {
                                    hasEmptyAdj = true;
                                }
                            }
                        }

                        if (hasEmptyAdj) {
                            std::vector<VertexData>& vboData = isTransparent(currentBlock) ? m_vboData.m_VBODataTransparent : m_vboData.m_VBODataOpaque;
                            std::vector<GLuint>& idxData = isTransparent(currentBlock) ? m_vboData.m_idxDataTransparent : m_vboData.m_idxDataOpaque;
                            unsigned int& numFaces = isTransparent(currentBlock) ? numTransparentFaces : numOpaqueFaces;

                            for (VertexData vert : neighbor.vertices) {
                                vert.pos += glm::vec4(x + this->minX, y, z + this->minZ, 0);
                                vert.nor = glm::vec4(neighbor.direction.x, neighbor.direction.y, neighbor.direction.z, 0);
                                vert.uv += blockFaceUVs.at(currentBlock).at(neighbor.d);
                                vert.animatable = isAnimatable(currentBlock) ? 1.f : 0.f;

                                vboData.push_back(vert);
                            }

                            for (int i : {0,1,2,0,2,3}) {
                                idxData.push_back(numFaces * 4 + i);
                            }
                            numFaces++;
                        }
                    }
                }
            }
        }
    }
}

void Chunk::createAndBufferVBOData() {
    createVBOdata();
    create(m_vboData.m_VBODataOpaque, m_vboData.m_idxDataOpaque, m_vboData.m_VBODataTransparent, m_vboData.m_idxDataTransparent);
}


void Chunk::create(std::vector<VertexData> &vboDataOpaque,
                   std::vector<GLuint> &idxOpaque,
                   std::vector<VertexData> &vboDataTransparent,
                   std::vector<GLuint> &idxTransparent) {

    indexCounts[INDEX] = idxOpaque.size();
    generateBuffer(INDEX);
    bindBuffer(INDEX);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxOpaque.size() * sizeof(GLuint), idxOpaque.data(), GL_STATIC_DRAW);

    generateBuffer(INTERLEAVED);
    bindBuffer(INTERLEAVED);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vboDataOpaque.size() * sizeof(VertexData), vboDataOpaque.data(), GL_STATIC_DRAW);

    indexCounts[INDEX_TRANS] = idxTransparent.size();
    generateBuffer(INDEX_TRANS);
    bindBuffer(INDEX_TRANS);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxTransparent.size() * sizeof(GLuint), idxTransparent.data(), GL_STATIC_DRAW);

    generateBuffer(INTERLEAVED_TRANS);
    bindBuffer(INTERLEAVED_TRANS);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vboDataTransparent.size() * sizeof(VertexData), vboDataTransparent.data(), GL_STATIC_DRAW);
}

GLenum Chunk::drawMode()
{
    return GL_TRIANGLES;
}

bool Chunk::isVBODataCreated() const {
    return vboDataCreated;
}

void Chunk::setVBODataCreated(bool v) {
    vboDataCreated = v;
}
