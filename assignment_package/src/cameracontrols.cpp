#include "player.h"

const static float SQUARED_3 = sqrt(3.f);

const static float DELTA_X[] = {-0.4f, -0.4f, 0.4f, 0.4f};
const static float DELTA_Z[] = {-0.4f, 0.4f, 0.4f, -0.4f};

// Processes input received from a mouse input system
void Player::processMouseMovement(float xOffset, float yOffset)
{
    cameraPtr->pitch += mouseSensitivity * yOffset;
    cameraPtr->yaw += mouseSensitivity * xOffset;

    if (cameraPtr->pitch > 89.f)
    {
        cameraPtr->pitch = 89.f;
    }
    if (cameraPtr->pitch < -89.f)
    {
        cameraPtr->pitch = -89.f;
    }

    if (cameraPtr->yaw > 360.f)
    {
        cameraPtr->yaw -= 360.f;
    }
    if (cameraPtr->yaw < - 360.f)
    {
        cameraPtr->yaw += 360.f;
    }

    cameraPtr->recomputeAttributes();
}

// Moves the camera according to the input keys and update position ignoring collision
void Player::flyWithoutCollisions()
{
    glm::vec3 displacement = velocity * deltaT;
    glm::vec3 expectedPos = cameraPtr->eye + displacement;

    setGlobalPosition(expectedPos);
    cameraPtr->recomputeAttributes();
}

// Moves the Player with camera in a free fall motion under weightlessness
void Player::freeFall(Terrain* terrain)
{
    glm::vec3 displacement = velocity * deltaT;
    glm::vec3 exptPos = position + displacement;
    glm::vec3 normalizedDisp = glm::normalize(displacement);
    bool isCollisionDetected = false;
    float maxDisp = glm::length(displacement);

    // Scans all the 4 vertices around the feet (the down layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(exptPos.x + DELTA_X[k], exptPos.y - 1.6f, exptPos.z + DELTA_Z[k]);
        if (collisionDetect(vertex, terrain))
        {
            isCollisionDetected = true;
            glm::vec3 preVertex = glm::vec3(position.x + DELTA_X[k], position.y - 1.6f, position.z + DELTA_Z[k]);
            float t = collisionDetect(preVertex, normalizedDisp, maxDisp, terrain);

            exptPos = position + t * normalizedDisp;
            if (velocity.y <= 0)
            {
                motionState = Motion::WALKING;
            }
            velocity.y = 0.f;
            break;
        }
    }

    if (velocity.y > 0)
    {
        // Scans all the 4 vertices around the feet (the down layer)
        for (int k = 0; k < 4; k++)
        {
            glm::vec3 vertex = glm::vec3(exptPos.x + DELTA_X[k], exptPos.y + 0.4f, exptPos.z + DELTA_Z[k]);
            if (collisionDetect(vertex, terrain))
            {
                isCollisionDetected = true;
                glm::vec3 preVertex = glm::vec3(position.x + DELTA_X[k], position.y + 0.4f, position.z + DELTA_Z[k]);
                float t = collisionDetect(preVertex, normalizedDisp, maxDisp, terrain);

                exptPos = position + t * normalizedDisp * 0.45f;
                if (velocity.y <= 0)
                {
                    motionState = Motion::WALKING;
                }
                velocity.y = 0.f;
                break;
            }
        }
    }

    setGlobalPosition(exptPos);
    cameraPtr->recomputeAttributes();
}

// Checks whether the Player's feet are on land or not
void Player::checkFeetOnLand(Terrain* terrain)
{
    bool aVertexOnLand = false;
    int verticesInLiquid = 0;
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 footVertex = glm::vec3(position.x + DELTA_X[k], position.y - 1.65f, position.z + DELTA_Z[k]);
        BlockType block = getTerrainBlockAt(footVertex, terrain);
        if (block != BlockType::EMPTY)
        {
            aVertexOnLand = true;
            if (block == BlockType::WATER || block == BlockType::LAVA)
                verticesInLiquid++;
        }
    }

    if (!aVertexOnLand)
    {
        motionState = Motion::WEIGHTLESSNESS;
    }
    if (verticesInLiquid == 4)
    {
        motionState = Motion::SWIMMING;
    }
}

// Moves the camera according to the input keys and update position without moving inside the world’s geometry
void Player::easyMoveWithCollisions(Terrain* terrain)
{
    glm::vec3 displacement = velocity * deltaT;
    glm::vec3 normalizedDisp = glm::normalize(displacement);
    glm::vec3 exptPos = position + displacement;

    float maxDisp = glm::length(displacement);
    if (fequal(maxDisp, 0.f))
    {
        // No movement
        return;
    }

    float t = 1.f;
    bool isCollisionDetected = false;
    // Scans all the 4 vertices around the head (the upper layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(exptPos.x + DELTA_X[k], exptPos.y + 0.4f, exptPos.z + DELTA_Z[k]);

        if (collisionDetect(vertex, terrain))
        {
            isCollisionDetected = true;
            glm::vec3 preVertex = glm::vec3(position.x + DELTA_X[k], position.y + 0.4f, position.z + DELTA_Z[k]);
            float tempT = collisionDetect(preVertex, normalizedDisp, maxDisp, terrain);
            t = min(t, tempT);
        }
    }

    // Scans all the 4 vertices around the waist (the middle layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(exptPos.x + DELTA_X[k], exptPos.y - 0.6f, exptPos.z + DELTA_Z[k]);

        if (collisionDetect(vertex, terrain))
        {
            isCollisionDetected = true;
            glm::vec3 preVertex = glm::vec3(position.x + DELTA_X[k], position.y - 0.6f, position.z + DELTA_Z[k]);
            float tempT = collisionDetect(preVertex, normalizedDisp, maxDisp, terrain);
            t = min(t, tempT);
        }
    }

    // Scans all the 4 vertices around the feet (the down layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(exptPos.x + DELTA_X[k], exptPos.y - 1.6f, exptPos.z + DELTA_Z[k]);

        if (collisionDetect(vertex, terrain))
        {
            isCollisionDetected = true;
            glm::vec3 preVertex = glm::vec3(position.x + DELTA_X[k], position.y - 1.6f, position.z + DELTA_Z[k]);
            float tempT = collisionDetect(preVertex, normalizedDisp, maxDisp, terrain);
            t = min(t, tempT);
        }
    }

    if (isCollisionDetected)
    {
        glm::vec3 newDisp = t * normalizedDisp;
        if (glm::length(newDisp) > maxDisp)
        {
            newDisp = displacement;
        }
        exptPos = position + newDisp * 0.45f;
    }

    setGlobalPosition(exptPos);
    cameraPtr->recomputeAttributes();
}

void Player::moveWithCollisions(Terrain* terrain)
{
    glm::vec3 displacement = velocity * deltaT;
    glm::vec3 currentPos = cameraPtr->eye;

    glm::vec3 normalizedDisp = glm::normalize(displacement);
    float maxDisp = glm::length(displacement);
    float t = FLT_MAX;
    // Scans all the 4 vertices around the head (the upper layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(currentPos.x + DELTA_X[k], currentPos.y, currentPos.z + DELTA_Z[k]);
        float tempT = collisionDetect(vertex, normalizedDisp, maxDisp, terrain);
        if (tempT < t)
        {
            t = tempT;
        }
    }

    // Scans all the 4 vertices around the waist (the middle layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(currentPos.x + DELTA_X[k], currentPos.y - 1, currentPos.z + DELTA_Z[k]);
        float tempT = collisionDetect(vertex, normalizedDisp, maxDisp, terrain);
        if (tempT < t)
        {
            t = tempT;
        }
    }

    // Scans all the 4 vertices around the feet (the down layer)
    for (int k = 0; k < 4; k++)
    {
        glm::vec3 vertex = glm::vec3(currentPos.x + DELTA_X[k], currentPos.y - 2, currentPos.z + DELTA_Z[k]);
        float tempT = collisionDetect(vertex, normalizedDisp, maxDisp, terrain);
        if (tempT < t)
        {
            t = tempT;
        }
    }

    if (maxDisp > glm::length(t * normalizedDisp))
        displacement = t * normalizedDisp;
    setGlobalPosition(currentPos + displacement);
    cameraPtr->recomputeAttributes();
}


// Returns true if the function detects there will be a collision between the Player and a certain block
bool Player::collisionDetect(const glm::vec3& detectedPos, const short axis, const bool isSameDirection, Terrain* terrain)
{
    if (axis == 1)
    {
        float displacement = isSameDirection ? 0.5f : -0.5f;
        glm::vec3 vert1 = detectedPos;
        glm::vec3 vert2 = detectedPos;
        vert1.x += displacement;
        vert2.x += displacement;
        vert1.z -= 0.5f;
        vert1.z += 0.5f;

        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain))
        {
            return true;
        }

        // The 2 detected vertices are in the air, no collision
        // However, we should check the middle layer and down layer
        vert1.y -= 1.f;
        vert2.y -= 1.f;
        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain))
        {
            return true;
        }

        vert1.y -= 1.f;
        vert2.y -= 1.f;
        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain))
        {
            if (motionState == Motion::WEIGHTLESSNESS)
            {
                motionState = Motion::WALKING;
            }
            return true;
        }

        return false;
    }

    if (axis == 2)
    {
        float displacement = isSameDirection ? 0.f : -2.f;
        float x = detectedPos.x;
        float y = detectedPos.y;
        float z = detectedPos.z;
        glm::vec3 vert1 = glm::vec3(x - 0.5f, y + displacement, z - 0.5f);
        glm::vec3 vert2 = glm::vec3(x + 0.5f, y + displacement, z - 0.5f);
        glm::vec3 vert3 = glm::vec3(x - 0.5f, y + displacement, z + 0.5f);
        glm::vec3 vert4 = glm::vec3(x + 0.5f, y + displacement, z + 0.5f);

        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain) ||
            !isVertexInEmptyBlock(vert3, terrain) || !isVertexInEmptyBlock(vert4, terrain))
        {
            if (motionState == Motion::WEIGHTLESSNESS)
            {
                motionState = Motion::WALKING;
            }
            return true;
        }

        return false;
    }

    if (axis == 3)
    {
        float displacement = isSameDirection ? 0.5f : -0.5f;
        glm::vec3 vert1 = detectedPos;
        glm::vec3 vert2 = detectedPos;
        vert1.z += displacement;
        vert2.z += displacement;
        vert1.x -= 0.5f;
        vert1.x += 0.5f;

        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain))
        {
            return true;
        }

        // The 2 detected vertices are in the air, no collision
        // However, we should check the middle layer and down layer
        vert1.y -= 1.f;
        vert2.y -= 1.f;
        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain))
        {
            return true;
        }

        vert1.y -= 1.f;
        vert2.y -= 1.f;
        if (!isVertexInEmptyBlock(vert1, terrain) || !isVertexInEmptyBlock(vert2, terrain))
        {
            if (motionState == Motion::WEIGHTLESSNESS)
            {
                motionState = Motion::WALKING;
            }
            return true;
        }

        return false;
    }

    return true;
}

// Returns true if detect there will be a collision between the Player and a certain block
bool Player::collisionDetect(const glm::vec3& expectedPos, Terrain* terrain)
{
    glm::vec3 curBlock = glm::floor(expectedPos);
    BlockType curBlockType = getTerrainBlockAt(curBlock, terrain);
    return !canPassThrough(curBlockType);
}

// Returns true if detect there will be a collision between the Player and a certain block, based on ray-marching
float Player::collisionDetect(const glm::vec3& curPos, const glm::vec3& rayDirection, const float maxDisp, Terrain* terrain)
{
    glm::vec3 curBlock = glm::floor(curPos);
    glm::vec3 rayOrigin = curPos;

    float t = 0.f;
    while (t < maxDisp)
    {
        float minT = SQUARED_3;
        int interfaceAxis = 0; // used to track the axis for which t is smallest

        for (int i = 0; i < 3; i++)
        {
            if (fequal(rayDirection[i], 0.f))
                continue;

            float offset = max(0.f, glm::sign(rayDirection[i]));
            int nextIntercept = curBlock[i] + offset;
            float axisT = (nextIntercept - curPos[i]) / rayDirection[i];

            if (axisT < minT)
            {
                minT = axisT;
                interfaceAxis = i;
            }
        }

        t += minT;
        rayOrigin += minT * rayDirection;

        glm::ivec3 offsetVec(0);
        offsetVec[interfaceAxis] = min(0.f, glm::sign(rayDirection[interfaceAxis]));
        curBlock = glm::floor(rayOrigin);
        curBlock.x += offsetVec.x;
        curBlock.y += offsetVec.y;
        curBlock.z += offsetVec.z;

        BlockType curBlockType = getTerrainBlockAt(curBlock, terrain);
        if (!canPassThrough(curBlockType))
        {
            return min(t, maxDisp);
        }
    }

    return maxDisp;
}

// Returns true if the given position is inside an Empty type block
bool Player::isVertexInEmptyBlock(const glm::vec3& pos, Terrain* terrain)
{
    glm::vec3 floor_pos = glm::floor(pos);
    return terrain->getBlockAt(floor_pos.x - terrain->offsetX * 16.0f,
                               floor_pos.y,
                               floor_pos.z - terrain->offsetZ * 16.0f) == BlockType::EMPTY;
}

// Returns the enum type of the Terrain block at the given position
BlockType Player::getTerrainBlockAt(const glm::vec3& pos, Terrain* terrain)
{
    return terrain->getBlockAt(pos.x - terrain->offsetX * 16.0f,
                               pos.y,
                               pos.z - terrain->offsetZ * 16.0f);
}

