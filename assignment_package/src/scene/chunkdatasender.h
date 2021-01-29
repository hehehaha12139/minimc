#ifndef CHUNKDATASENDER_H
#define CHUNKDATASENDER_H

#include <QMutex>
#include <QRunnable>
#include "terrain.h"

class ChunkDataSender : public QRunnable
{
private:
    QMutex* mutexPtr;
    Chunk* chunkPtr;

public:
    ChunkDataSender(QMutex* qmPtr, Chunk* cPtr);
    void run() override;
};


#endif // CHUNKDATASENDER_H
