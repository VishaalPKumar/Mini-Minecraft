#include "vboworker.h"

VBOWorker::VBOWorker(Chunk* c, QMutex* mutex, std::vector<ChunkVBOData> &v) :
    chunk(c), mutex(mutex), chunksThatNeedVBOs(v)
{}

void VBOWorker::run() {
    chunk->createVBOdata();
    ChunkVBOData vboData = chunk->m_vboData;
    mutex->lock();
    chunksThatNeedVBOs.push_back(vboData);
    mutex->unlock();
}
