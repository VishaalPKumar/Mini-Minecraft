#include "blocktypeworker.h"
#include "scene/terrain.h"
#include "utils/noise.h"
#include "iostream"

constexpr float MIN_GENERATION_HEIGHT = 128.f;
constexpr float WATER_MAX_HEIGHT = 138.f;
constexpr float CAVES_MAX_HEIGHT = 128.f;
constexpr float LAVA_MAX_HEIGHT = 25.f;
constexpr float CAVE_SCALE = 10.f;

float mesaTerrainMaxHeight(glm::vec2 xz) {
    float h = 0;
    float amp = 0.8;
    float freq = 100;
    for(int i = 0; i < 4; ++i) {
        glm::vec2 offset = glm::vec2(fbm(xz / 256.f), fbm(xz / 300.f)) + glm::vec2(1000);
        float h1 = PerlinNoise2D((xz + offset * 75.f) / freq);
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }
    float s = glm::smoothstep(0.2f, 0.3f, abs(h)) * 0.85 + 0.15 * h;
    return 150 + floor(s * 40);
}

float mountainTerrainMaxHeight(glm::vec2 xz) {
    float h = 0;
    float amp = 0.5;
    float freq = 512;
    for(int i = 0; i < 4; ++i) {
        glm::vec2 offset = glm::vec2(fbm(xz / 150.f), fbm(xz / 256.f)) + glm::vec2(1000);
        float h1 = abs(PerlinNoise2D((xz + offset * 75.f) / freq));
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }
    h = 1 - h;
    float sm = glm::smoothstep(0.4f, 1.f, h * h * h);

    return 118 + floor((sm * 0.7 + h * h * h * 0.3) * 127);
}

float globalInterpPercentage(glm::vec2 xz) {
    return glm::smoothstep(0.15f, 0.25f, abs(PerlinNoise2D(xz / 500.f)));
}

float caveInterpPercentage(glm::vec3 xyz) {
    return PerlinNoise3D(xyz / CAVE_SCALE);
}

float worldTerrainMaxH(glm::vec2 xz, float interpPercent) {
    float mountain = mountainTerrainMaxHeight(xz);
    float mesa = mesaTerrainMaxHeight(xz);
    return floor(mountain * interpPercent + mesa * (1 - interpPercent));
}

float Terrain::maxHeight(int x, int z) {
    glm::vec2 xz = glm::vec2(x, z);
    return worldTerrainMaxH(xz, globalInterpPercentage(xz));
}

BlockType mountainBlockTypeAt(float currH, float maxH) {
    if (maxH > 200 && currH == maxH - 1) {
        return SNOW;
    }
    return STONE;
}

BlockType mesaBlockTypeAt(float currH, float maxH) {
    if (currH > 150) {
        return DIRT;
    }
    return SAND;
}

BlockType blockTypeAt(float currH, float maxH, float biomInterpPercentage) {
    if (currH == 0) {
        return BEDROCK;
    }
    if (currH < MIN_GENERATION_HEIGHT) {
        return STONE;
    }
    if (currH > maxH && currH <= WATER_MAX_HEIGHT) {
        return WATER;
    }
    if (biomInterpPercentage > 0.3) {
        return mountainBlockTypeAt(currH, maxH);
    } else {
        return mesaBlockTypeAt(currH, maxH);
    }
}

BlockType blockTypeAt(float currH, float maxH, float biomInterpPercentage, float caveInterpPercentage) {

    if (caveInterpPercentage < 0) {
        if (currH < LAVA_MAX_HEIGHT) {
            return LAVA;
        }
        return EMPTY;
    }
    if (currH == 0) {
        return BEDROCK;
    }

    return blockTypeAt(currH, maxH, biomInterpPercentage);
}

BlockTypeWorker::BlockTypeWorker(int minX, int minZ, Chunk* chunk, std::vector<Chunk*> &chunks, QMutex* mutex)
    : minX(minX), minZ(minZ), chunk(chunk), generatedChunks(chunks), mutex(mutex)
{}

void BlockTypeWorker::run() {
    for (int x = minX; x < minX + 16; ++x) {
        for (int z = minZ; z < minZ + 16; ++z) {
            glm::vec2 xz = glm::vec2(x, z);
            float biomInterpPercentage = globalInterpPercentage(xz);
            float maxH = worldTerrainMaxH(xz, biomInterpPercentage);
            int localX = x - minX;
            int localZ = z - minZ;
            for (int h = 0; h < glm::max(maxH, WATER_MAX_HEIGHT); h++) {
                if (1 <= h && h <= CAVES_MAX_HEIGHT) {
                    float caveInterp = caveInterpPercentage(glm::vec3 (x, h, z));
                    chunk->setLocalBlockAt(localX, h, localZ, blockTypeAt(h, maxH, biomInterpPercentage, caveInterp));
                } else {
                    chunk->setLocalBlockAt(localX, h, localZ, blockTypeAt(h, maxH, biomInterpPercentage));
                }
            }
        }
    }
    mutex->lock();
    generatedChunks.push_back(chunk);
    mutex->unlock();
}
