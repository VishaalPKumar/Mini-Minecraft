#include "terrain.h"
#include "cube.h"
#include "utils/noise.h"
#include "workers/blocktypeworker.h"
#include "workers/vboworker.h"
#include <stdexcept>
#include <iostream>
#include <QThreadPool>

int Terrain::RENDER_DISTANCE = 12; // (# of chunks)
int Terrain::CHUNK_LENGTH = 16;
int Terrain::LOAD_DISTANCE = 3; // (# of zones)
int Terrain::ZONE_SIZE = 4; // (# of chunks)

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_geomCube(context),
    mp_context(context), m_chunksThatHaveBlockDataLock(), m_chunksThatHaveBlockData()
{}

Terrain::~Terrain() {
    // m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getGlobalBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                                  static_cast<unsigned int>(y),
                                  static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getGlobalBlockAt(glm::vec3 p) const {
    return getGlobalBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setGlobalBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                           static_cast<unsigned int>(y),
                           static_cast<unsigned int>(z - chunkOrigin.y),
                           t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = std::move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

void Terrain::spawnBlockTypeThread(int x, int z, Chunk* c) {
    BlockTypeWorker *worker = new BlockTypeWorker(x, z, c, m_chunksThatHaveBlockData, &m_chunksThatHaveBlockDataLock);
    QThreadPool::globalInstance()->start(worker);
}

void Terrain::spawnVBOThread(Chunk* c) {
    VBOWorker *worker = new VBOWorker(c, &m_chunksThatHaveVBOsLock, m_chunksThatHaveVBOs);
    QThreadPool::globalInstance()->start(worker);
}

void Terrain::generateTerrainZone(int minX, int minZ) {

    // Insert the terrain zone if it doesn't already exist
    if (m_generatedTerrain.find(toKey(minX, minZ)) == m_generatedTerrain.end()) {
        m_generatedTerrain.insert(toKey(minX, minZ));
    }

    // Instantiate and fill the chunks in this zone
    for (int x = minX; x <= minX + (ZONE_SIZE * 16); x += 16) {
        for (int z = minZ; z <= minZ + (ZONE_SIZE * 16); z += 16) {
            if (!hasChunkAt(x, z)) {
                Chunk* c = instantiateChunkAt(x, z);
                spawnBlockTypeThread(x, z, c);
            }
        }
    }

    // Generate VBO data for generated chunks
    m_chunksThatHaveBlockDataLock.lock();
    for (Chunk* c : m_chunksThatHaveBlockData) {
        spawnVBOThread(c);
    }
    m_chunksThatHaveBlockData.clear();
    m_chunksThatHaveBlockDataLock.unlock();

    m_chunksThatHaveVBOsLock.lock();
    for (ChunkVBOData &cd : m_chunksThatHaveVBOs) {
        cd.mp_chunk->create(cd.m_VBODataOpaque, cd.m_idxDataOpaque,
                            cd.m_VBODataTransparent, cd.m_idxDataTransparent);
        cd.mp_chunk->setVBODataCreated(true);
    }
    m_chunksThatHaveVBOs.clear();
    m_chunksThatHaveVBOsLock.unlock();
}

void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            if(chunk && chunk->isVBODataCreated() && chunk->elemCount(INDEX) > 0) {
                shaderProgram->drawOpaque(*chunk);
            }
        }
    }

    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            if(chunk && chunk->isVBODataCreated() && chunk->elemCount(INDEX_TRANS) > 0) {
                shaderProgram->drawTransparent(*chunk);
            }
        }
    }
}

void Terrain::CreateInitialScene()
{
    int xMin = -LOAD_DISTANCE * ZONE_SIZE * 16;
    int xMax = LOAD_DISTANCE * ZONE_SIZE * 16;
    int zMin = -LOAD_DISTANCE * ZONE_SIZE * 16;
    int zMax = LOAD_DISTANCE * ZONE_SIZE * 16;

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = xMin; x <= xMax; x += (16 * ZONE_SIZE)) {
        for(int z = zMin; z <= zMax; z += (16 * ZONE_SIZE)) {
            generateTerrainZone(x, z);
        }
    }
}

void Terrain::updateLoadedChunks(int playerX, int playerZ) {

    glm::vec2 player_pos = glm::vec2(floor(playerX / (ZONE_SIZE*16.)) * (ZONE_SIZE*16.),
                                     floor(playerZ / (ZONE_SIZE*16.)) * (ZONE_SIZE*16.));

    int xMin = player_pos.x -LOAD_DISTANCE * ZONE_SIZE * 16;
    int xMax = player_pos.x + LOAD_DISTANCE * ZONE_SIZE * 16;
    int zMin = player_pos.y -LOAD_DISTANCE * ZONE_SIZE * 16;
    int zMax = player_pos.y + LOAD_DISTANCE * ZONE_SIZE * 16;

    for (int x = xMin; x <= xMax; x += (16 * ZONE_SIZE)) {
        for (int z = zMin; z <= zMax; z += (16 * ZONE_SIZE)) {
            generateTerrainZone(x, z);
        }
    }
}
