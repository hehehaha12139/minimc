#include "playerstatusinspector.h"
#include "utils.h"

PlayerStatusInspector::PlayerStatusInspector(Player* pPtr, Terrain* tPtr)
    : playerPtr(pPtr), terrainPtr(tPtr)
{}

void PlayerStatusInspector::run()
{
    while (true)
    {
        if (playerPtr->isKeyAPressed() || playerPtr->isKeyDPressed() || playerPtr->isKeySPressed() || playerPtr->isKeyWPressed() ||
            playerPtr->isKeyQPressed() || playerPtr->isKeyEPressed() || playerPtr->isSpaceBarPressed())
        {
            playerPtr->setMovingStatus(true);
        }
        else
        {
            playerPtr->setMovingStatus(false);
        }

        if (playerPtr->getMotionState() == Motion::WALKING || playerPtr->getMotionState() == Motion::SWIMMING)
        {
            playerPtr->checkFeetOnLand(terrainPtr);
        }
    }
}
