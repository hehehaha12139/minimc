#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <postprocessshader.h>
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include <scene/camera.h>
#include <scene/terrain.h>
#include "scene/overlayquad.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QSound>
#include <QMutex>
#include <QThreadPool>
#include <smartpointerhelp.h>
#include "player.h"
#include "playerstatusinspector.h"
#include <array>
#include <set>
#include <texture.h>


class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    uPtr<WorldAxes> mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    uPtr<ShaderProgram> mp_progLambert;// A shader program that uses lambertian reflection
    uPtr<ShaderProgram> mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    uPtr<ShaderProgram> mp_progLiquid;// A shader program used for water and lava overlay
    uPtr<ShaderProgram> mp_progShadow; // A shader program that uses shadow mapping
    uPtr<ShaderProgram> mp_progDepth; // A shader program rendering shadow texture
//    uPtr<PostProcessShader> mp_progPostprocess;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    // A collection of handles to the five frame buffers we've given
    // ourselves to perform render passes. The 0th frame buffer is always
    // written to by the render pass that uses the currently bound surface shader.
    GLuint m_frameBuffer;
    // A collection of handles to the textures used by the frame buffers.
    // m_frameBuffers[i] writes to m_renderedTextures[i].
    GLuint m_renderedTexture;
    // A collection of handles to the depth buffers used by our frame buffers.
    // m_frameBuffers[i] writes to m_depthRenderBuffers[i].
    GLuint m_depthRenderBuffer;

    // Frame buffer for shadow mapping
    GLuint m_shadowMappingFBO;
    GLuint m_depthMap;

    uPtr<Player> mp_player; // Player
    uPtr<Camera> mp_camera;
    uPtr<Terrain> mp_terrain;
    uPtr<TestCubeSet> mp_testCubeSet;
    uPtr<PlayerStatusInspector> mp_playerStatusInspector;

    uPtr<SketchCube> mp_sketchCube;
    glm::vec3 cameraAlignedPosition;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer m_timer;

    uPtr<QThreadPool> mp_threadPool;
    QMutex m_mutex;

    uPtr<QSound> mp_bgmSound;
    uPtr<QSound> mp_skySound;
    uPtr<QSound> mp_swimSound;
    uPtr<QSound> mp_underwaterSound;
    uPtr<QSound> mp_walkSound;
    uPtr<QSound> mp_flySound;
    QSound* mp_curBgmSound;
    int64_t lastUpdateTime;

    bool lockMouse;
    std::vector<glm::vec3> testCubePositions;
    std::vector<int> testCubeTypes;
    std::unique_ptr<Texture> testCubeTexture;

    void updateKeyboardControl();

    // Overlays for water and lava blocks
    uPtr<OverlayQuad> mp_waterOverlay;
    uPtr<OverlayQuad> mp_lavaOverlay;

    // A helper function that iterates through
    // each of the render passes required by the
    // currently bound post-process shader and
    // invokes them.
    void performPostprocessRenderPass();

    // Sets up the arrays of frame buffers
    // used to store render passes. Invoked
    // once in initializeGL().
//    void createRenderBuffers();

    int m_time;

    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    // draw every chunk
    void GLDrawScene();

    // draw shadow texture
    void GLDrawShadow();

    // silky smooth keyboard control;
    std::array<bool, 256> keyboardAllStars;
    int64_t getElapsedTime();

    void GLDrawTestCubes();
    void updateSketchCube();
    void updateCameraAlignedPosition();

    bool enableMovingCastle;

    void setBackgroundMusic();

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);
    void mouseMoveEvent(QMouseEvent *e);


private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
