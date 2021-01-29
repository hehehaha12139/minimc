#include "player.h"
#include "utils.h"
#include <QDebug>

const static float LIQUID_DELAY_RATE = 0.36f;

// Contructor for Player class
Player::Player(Camera* cPtr)
    : cameraPtr(cPtr),
      position(glm::vec3(0.f)),
      velocity(glm::vec3(0.f)),
      speed(4.5f),
      deltaT(0.f),
      wKeyPressed(false),
      aKeyPressed(false),
      sKeyPressed(false),
      dkeyPressed(false),
      qKeyPressed(false),
      eKeyPressed(false),
      spacePressed(false),
      willingToMove(false),
      cursorDeltaPos(glm::vec2()),
      mouseLeftBtnState(MouseState::STILL),
      mouseRightBtnState(MouseState::STILL),
      motionState(Motion::FLYING),
      mouseSensitivity(0.05f)
{}

void Player::listenKeyPressEvent(QKeyEvent* e, const bool isKeyPressed)
{
    switch (e->key())
    {
    case Qt::Key_A:
    {
        if (isKeyPressed)
        {
            aKeyPressed = true;
        }
        else
        {
            aKeyPressed = false;
        }
        break;
    }
    case Qt::Key_D:
    {
        if (isKeyPressed)
        {
            dkeyPressed = true;
        }
        else
        {
            dkeyPressed = false;
        }
        break;
    }
    case Qt::Key_S:
    {
        if (isKeyPressed)
        {
            sKeyPressed = true;
        }
        else
        {
            sKeyPressed = false;
        }
        break;
    }
    case Qt::Key_W:
    {
        if (isKeyPressed)
        {
            wKeyPressed = true;
        }
        else
        {
            wKeyPressed = false;
        }
        break;
    }
    case Qt::Key_Space:
    {
        if (isKeyPressed)
        {
            spacePressed = true;
        }
        else
        {
            spacePressed = false;
        }
        break;
    }
    case Qt::Key_F:
    {
        if (isKeyPressed)
        {
            if (motionState != Motion::FLYING)
            {
                fly();
            }
            else
            {
                motionState = Motion::WEIGHTLESSNESS;
            }
        }

        break;
    }
    case Qt::Key_Q:
    {
        if (isKeyPressed)
        {
            qKeyPressed = true;
        }
        else
        {
            qKeyPressed = false;
        }
        break;
    }
    case Qt::Key_E:
    {
        if (isKeyPressed)
        {
            eKeyPressed = true;
        }
        else
        {
            eKeyPressed = false;
        }
        break;
    }
    default:
        break;
    }
}

void Player::listenMouseEvent(QMouseEvent* e, const MouseState state)
{
    if (state == MouseState::MOVED)
    {
        QPoint new_pos = e->pos();
        float xOffset = new_pos.x() - cameraPtr->width / 2.f;
        float yOffset = cameraPtr->height / 2.f - new_pos.y();
        processMouseMovement(xOffset, yOffset);

        cursorDeltaPos.x = xOffset;
        cursorDeltaPos.y = yOffset;
        return;
    }

    switch (e->button())
    {
    case Qt::LeftButton:
    {
        if (state == MouseState::PRESSED)
        {
            mouseLeftBtnState = PRESSED;
        }
        else
        {
            mouseLeftBtnState = RELEASED;
        }
        break;
    }
    case Qt::RightButton:
    {
        if (state == MouseState::PRESSED)
        {
            mouseRightBtnState = PRESSED;
        }
        else
        {
            mouseRightBtnState = RELEASED;
        }
        break;
    }
    default:
        break;
    }
}

// Setters for Player class
void Player::setGlobalPosition(const glm::vec3& pos)
{
    position = pos;
    cameraPtr->eye = pos;
}

void Player::setGlobalVelocity(const glm::vec3& v)
{
    velocity = v;
}

void Player::setCameraPtr(Camera* cPtr)
{
    cameraPtr = cPtr;
}

void Player::setMovingStatus(const bool status)
{
    willingToMove = status;
}

// Getters for Player class
glm::vec3 Player::getGlobalPosition() const
{
    return position;
}

glm::vec3 Player::getGlobalVelocity() const
{
    return velocity;
}

Camera* Player::getCameraPtr() const
{
    return cameraPtr;
}

bool Player::isKeyWPressed() const
{
    return wKeyPressed;
}

bool Player::isKeyAPressed() const
{
    return aKeyPressed;
}

bool Player::isKeySPressed() const
{
    return sKeyPressed;
}

bool Player::isKeyDPressed() const
{
    return dkeyPressed;
}

bool Player::isKeyQPressed() const
{
    return qKeyPressed;
}

bool Player::isKeyEPressed() const
{
    return eKeyPressed;
}

bool Player::isSpaceBarPressed() const
{
    return spacePressed;
}

bool Player::isWillingToMove() const
{
    return willingToMove;
}

glm::vec2 Player::getCursorDeltaPos() const
{
    return cursorDeltaPos;
}

MouseState Player::getMouseLeftBtnState() const
{
    return mouseLeftBtnState;
}

MouseState Player::getMouseRightBtnState() const
{
    return mouseRightBtnState;
}

Motion Player::getMotionState() const
{
    return motionState;
}


// Updates the elapsed time
void Player::updateDeltaTime(float timeInMS)
{
    deltaT = timeInMS * 0.001f;
}

void Player::updateVelocity()
{
    switch (motionState)
    {
    case Motion::FLYING:
        updateVelocityWhenFlying();
        break;
    case Motion::WALKING:
        updateVelocityWhenWalking();
        break;
    case Motion::WEIGHTLESSNESS:
        updateVelocityWhenWeightlessness();
        break;
    case Motion::SWIMMING:
        updateVelocityWhenSwimming();
        break;
    default:
        break;
    }
}

// Updates the velocity vector when the player is flying
void Player::updateVelocityWhenFlying()
{
    glm::vec3 direction(0.f);
    if (wKeyPressed)
    {
        direction.z += 1.f;
    }

    if (aKeyPressed)
    {
        direction.x -= 1.f;
    }

    if (sKeyPressed)
    {
        direction.z -= 1.f;
    }

    if (dkeyPressed)
    {
        direction.x += 1.f;
    }

    if (qKeyPressed)
    {
        direction.y -= 1.f;
    }

    if (eKeyPressed)
    {
        direction.y += 1.f;
    }

    glm::vec3 globalDirection = cameraPtr->right * direction.x + cameraPtr->world_up * direction.y + cameraPtr->look * direction.z;
    if(fequal(direction.y, 0.f))
    {
        globalDirection.y = 0.f;
    }

    if (!fequal(glm::length(globalDirection), 0.f))
    {
        velocity = glm::normalize(globalDirection) * speed * 3.6f;
    }
    else
    {
        velocity.x = 0.f;
        velocity.y = 0.f;
        velocity.z = 0.f;
    }
}

// Updates the velocity vector when the player is walking
void Player::updateVelocityWhenWalking()
{
    glm::vec3 direction(0.f);
    if (wKeyPressed)
    {
        direction.z += 1.f;
    }

    if (aKeyPressed)
    {
        direction.x -= 1.f;
    }

    if (sKeyPressed)
    {
        direction.z -= 1.f;
    }

    if (dkeyPressed)
    {
        direction.x += 1.f;
    }

    glm::vec3 globalDirection = cameraPtr->right * direction.x + cameraPtr->world_up * direction.y + cameraPtr->look * direction.z;
    globalDirection.y = 0.f;

    if (!fequal(glm::length(globalDirection), 0.f))
    {
        velocity = glm::normalize(globalDirection) * speed;
    }
    else
    {
        velocity.x = 0.f;
        velocity.y = 0.f;
        velocity.z = 0.f;
    }

    if (spacePressed)
    {
        // jump
        velocity.y = 5.f;
        velocity.x = velocity.z = 0.f;
    }
}

// Updates the velocity vector when the player is weightless
void Player::updateVelocityWhenWeightlessness()
{
    if (velocity.y <= 0)
    {
        velocity.x *= 0.85f;
        velocity.z *= 0.85f;
    }
    else
    {
        glm::vec3 direction(0.f);
        if (wKeyPressed)
        {
            direction.z += 1.f;
        }

        if (aKeyPressed)
        {
            direction.x -= 1.f;
        }

        if (sKeyPressed)
        {
            direction.z -= 1.f;
        }

        if (dkeyPressed)
        {
            direction.x += 1.f;
        }

        glm::vec3 globalDirection = cameraPtr->right * direction.x + cameraPtr->look * direction.z;
        if (!fequal(glm::length(globalDirection), 0.f))
        {
            globalDirection = glm::normalize(globalDirection) * speed * 0.9f;
            velocity.x = globalDirection.x;
            velocity.z = globalDirection.z;
        }
    }

    velocity.y -= 10.f * deltaT;
}

// Updates the velocity vector when the player is swimming
void Player::updateVelocityWhenSwimming()
{
    glm::vec3 direction(0.f);
    if (wKeyPressed)
    {
        direction.z += 1.f;
    }

    if (aKeyPressed)
    {
        direction.x -= 1.f;
    }

    if (sKeyPressed)
    {
        direction.z -= 1.f;
    }

    if (dkeyPressed)
    {
        direction.x += 1.f;
    }

    if (spacePressed)
    {
        direction.y += 0.25f;
    }

    glm::vec3 globalDirection = cameraPtr->right * direction.x + cameraPtr->world_up * direction.y + cameraPtr->look * direction.z;
    if (!fequal(glm::length(globalDirection), 0.f))
    {
        velocity = glm::normalize(globalDirection) * speed * LIQUID_DELAY_RATE;
        if (fequal(velocity.x, 0.f) && fequal(velocity.z, 0.f))
            velocity.y *= 0.75f;
    }

    velocity.y -= LIQUID_DELAY_RATE * 1.5f * deltaT;
}

// Makes the player fly and ignore gravity
void Player::fly()
{
    motionState = Motion::FLYING;
    position.y = 255.f;
    velocity.y = 0.f;
    setGlobalPosition(position);
    cameraPtr->recomputeAttributes();
}

// Determines a block can be passed through or not
bool Player::canPassThrough(const BlockType block)
{
    if (block == BlockType::EMPTY || block == BlockType::WATER || block == BlockType::LAVA)
        return true;
    return false;
}

// Determines the camera (or head) is submerged in water (or lava) or not
bool Player::isSubmergedInLiquid(Terrain* terrain)
{
    glm::vec3 floorEye = glm::floor(position);
    BlockType block = getTerrainBlockAt(floorEye, terrain);
    return (block == BlockType::WATER || block == BlockType::LAVA);
}

// Gets the block where the camera is located
BlockType Player::getCameraLocatedBlock(Terrain* terrain)
{
    if (cameraPtr == nullptr)
        return BlockType::EMPTY;
    glm::vec3 floorEye = glm::floor(cameraPtr->eye);
    return getTerrainBlockAt(floorEye, terrain);
}
