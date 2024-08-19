#ifndef VBOWORKER_H
#define VBOWORKER_H
#include <QRunnable>
#include <QMutex>
#include "scene/chunk.h"

class VBOWorker : public QRunnable
{
private:
    Chunk* chunk;
    QMutex* mutex;
    std::vector<ChunkVBOData> &chunksThatNeedVBOs;
public:
    VBOWorker(Chunk*, QMutex*, std::vector<ChunkVBOData>&);
    void run() override;
};

#endif // VBOWORKER_H
