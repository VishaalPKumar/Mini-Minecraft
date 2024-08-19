#ifndef BLOCKTYPEWORKER_H
#define BLOCKTYPEWORKER_H
#include <QRunnable>
#include <QMutex>
#include "scene/chunk.h"

class BlockTypeWorker : public QRunnable
{
private:
    int minX;
    int minZ;
    Chunk* chunk;
    std::vector<Chunk*> &generatedChunks;
    QMutex* mutex;
public:
    BlockTypeWorker(int, int, Chunk*, std::vector<Chunk*>&, QMutex*);
    void run() override;
};

#endif // BLOCKTYPEWORKER_H
