#include "chunkdatasender.h"

ChunkDataSender::ChunkDataSender(QMutex* qmPtr, Chunk* cPtr)
    : mutexPtr(qmPtr), chunkPtr(cPtr)
{}

void ChunkDataSender::run()
{
    chunkPtr->sendGPUDirectives(mutexPtr);
}
