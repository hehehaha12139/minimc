#ifndef TERRAINCREATOR_H
#define TERRAINCREATOR_H

#include <QRunnable>
#include <QMutex>
#include "terrain.h"
#include "shaderprogram.h"

class TerrainCreator : public QRunnable
{
private:
    QMutex* mutexPtr;
    Terrain* terrainPtr;
    int index;
public:
    TerrainCreator(QMutex* qmPtr, Terrain* tPtr, int i);
    void run() override;
};

class TerrainUpdater : public QRunnable
{
private:
    QMutex* mutexPtr;
    Terrain* terrainPtr;
    int index;
public:
    TerrainUpdater(QMutex* qmPtr, Terrain* tPtr, int i);
    void run() override;
};

#endif // TERRAINCREATOR_H
