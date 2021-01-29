#include "terraincreator.h"
#include <QTime>

TerrainCreator::TerrainCreator(QMutex* qmPtr, Terrain* tPtr, int i)
    : mutexPtr(qmPtr), terrainPtr(tPtr), index(i)
{}

void TerrainCreator::run()
{
    // Not use mutexPtr->lock();
    // Because all the threads generating terrain according to index
    // The thread pool of MyGL will wait all threads' work done to replace mutex's job
    terrainPtr->generateTerrainAt(index);
}

TerrainUpdater::TerrainUpdater(QMutex *qmPtr, Terrain *tPtr, int i)
    : mutexPtr(qmPtr), terrainPtr(tPtr), index(i)
{}

void TerrainUpdater::run()
{
    // Not use mutexPtr->lock();
    // Because all the threads generating terrain according to index
    // The thread pool of MyGL will wait all threads' work done to replace mutex's job
    terrainPtr->updateChunkAt(index);
}
