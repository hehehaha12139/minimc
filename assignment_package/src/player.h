#ifndef PLAYER_H
#define PLAYER_H
#include <la.h>
#include <QDebug>
#include <QKeyEvent>
#include <QWidget>
#include <QSound>
#include "scene/camera.h"
#include "scene/terrain.h"
#include "utils.h"

enum MouseState {STILL, MOVED, PRESSED, RELEASED, DOUBLE_CLICKED, SCROLLED};
enum Motion {WALKING, FLYING, WEIGHTLESSNESS, SWIMMING};

class Player
{
private:
    // The pointer to a Camera
    Camera* cameraPtr;
    // The position in 3D space
    glm::vec3 position;
    // The velocity in 3D space
    glm::vec3 velocity;
    // The scaler representation of velocity
    float speed;
    // The time elapsed since the last update call
    float deltaT;

    // A set of variables to track the relevant inputs from the mouse and keyboard
    bool wKeyPressed;
    bool aKeyPressed;
    bool sKeyPressed;
    bool dkeyPressed;
    bool qKeyPressed;
    bool eKeyPressed;
    bool spacePressed;
    bool willingToMove;

    // The vairable to track the change in the cursor's X and Y coordinates
    glm::vec2 cursorDeltaPos;
    MouseState mouseLeftBtnState;
    MouseState mouseRightBtnState;

    Motion motionState;

    // The variable to constrol the sensitivity of mouse
    float mouseSensitivity;

public:
    // Constructor
    Player(Camera* cPtr = nullptr);

    void listenKeyPressEvent(QKeyEvent* e, const bool isKeyPressed);
    void listenMouseEvent(QMouseEvent* e, const MouseState state);

    // Setters for Player class
    void setGlobalPosition(const glm::vec3& pos);
    void setGlobalVelocity(const glm::vec3& v);
    void setCameraPtr(Camera* c_ptr);
    void setMovingStatus(const bool status);

    // Getters for Player class
    glm::vec3 getGlobalPosition() const;
    glm::vec3 getGlobalVelocity() const;
    Camera* getCameraPtr() const;
    bool isKeyWPressed() const;
    bool isKeyAPressed() const;
    bool isKeySPressed() const;
    bool isKeyDPressed() const;
    bool isKeyQPressed() const;
    bool isKeyEPressed() const;
    bool isSpaceBarPressed() const;
    bool isWillingToMove() const;
    glm::vec2 getCursorDeltaPos() const;
    MouseState getMouseLeftBtnState() const;
    MouseState getMouseRightBtnState() const;
    Motion getMotionState() const;

    // Processes input received from a mouse input system
    void processMouseMovement(float xOffset, float yOffset);

    // Updates the elapsed time
    void updateDeltaTime(float timeInMS);

    // Computes the velocity vector according to the input
    void updateVelocity();

    // Updates the velocity vector when the player is flying
    void updateVelocityWhenFlying();

    // Updates the velocity vector when the player is walking
    void updateVelocityWhenWalking();

    // Updates the velocity vector when the player is weightless
    void updateVelocityWhenWeightlessness();

    // Updates the velocity vector when the player is swimming
    void updateVelocityWhenSwimming();

    // Moves the camera according to the input keys and update position without moving inside the world’s geometry
    void moveWithCollisions(Terrain* terrain);

    // Moves the camera according to the input keys and update position without moving inside the world’s geometry
    void easyMoveWithCollisions(Terrain* terrain);

    // Moves the camera according to the input keys and update position ignoring collision
    void flyWithoutCollisions();

    // Moves the Player with camera in a free fall motion under weightlessness
    void freeFall(Terrain* terrain);

    // Checks whether the Player's feet are on land or not
    void checkFeetOnLand(Terrain* terrain);

    // Returns true if detect there will be a collision between the Player and a certain block
    bool collisionDetect(const glm::vec3& expectedPos, const short axis, const bool isSameDirection, Terrain* terrain);

    // Returns true if detect there will be a collision between the Player and a certain block
    bool collisionDetect(const glm::vec3& expectedPos, Terrain* terrain);

    // Returns true if detect there will be a collision between the Player and a certain block, based on ray-marching
    float collisionDetect(const glm::vec3& curPos, const glm::vec3& rayDirection, const float maxDisp, Terrain* terrain);

    // Returns true if the given position is inside an Empty type block
    bool isVertexInEmptyBlock(const glm::vec3& pos, Terrain* terrain);

    // Returns the enum type of the Terrain block at the given position
    BlockType getTerrainBlockAt(const glm::vec3& pos, Terrain* terrain);

    // Makes the player fly and ignore gravity
    void fly();

    // Determines a block can be passed through or not
    bool canPassThrough(const BlockType block);

    // Determines the camera (or head) is submerged in water (or lava) or not
    bool isSubmergedInLiquid(Terrain* terrain);

    // Gets the block where the camera is located
    BlockType getCameraLocatedBlock(Terrain* terrain);
};

#endif // PLAYER_H
