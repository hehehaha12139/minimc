#ifndef PLAYERSTATUSINSPECTOR_H
#define PLAYERSTATUSINSPECTOR_H

#include <QThread>
#include "player.h"
#include "scene/terrain.h"


class PlayerStatusInspector : public QThread
{
private:
    Player* playerPtr;
    Terrain* terrainPtr;

public:
    PlayerStatusInspector(Player* pPtr, Terrain* tPtr);
    void run() override;
};

#endif // PLAYERSTATUSINSPECTOR_H
