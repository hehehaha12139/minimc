#include "mygl.h"
#include "scene/terraincreator.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_worldAxes(mkU<WorldAxes>(this)),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)), mp_progLiquid(mkU<ShaderProgram>(this)),
      mp_camera(mkU<Camera>()), mp_terrain(mkU<Terrain>(this)), mp_player(mkU<Player>(mp_camera.get())),
      mp_threadPool(QThreadPool::globalInstance()),
      m_frameBuffer(-1), m_renderedTexture(-1), m_depthRenderBuffer(-1),
      mp_testCubeSet(mkU<TestCubeSet>(this, testCubePositions, testCubeTypes)),
      testCubeTexture(mkU<Texture>(this)),
      cameraAlignedPosition(mp_camera->eye),
      mp_sketchCube(mkU<SketchCube>(this, cameraAlignedPosition)),
      lockMouse(true), m_time(0.f), enableMovingCastle(false),
      mp_waterOverlay(mkU<OverlayQuad>(this, 'b')), mp_lavaOverlay(mkU<OverlayQuad>(this, 'r')),
      mp_swimSound(mkU<QSound>("qrc:/music/swim.wav")),
      mp_walkSound(mkU<QSound>("qrc:/music/walk.wav")),
      mp_bgmSound(mkU <QSound>("qrc:/music/Minecraft.wav")),
      mp_skySound(mkU<QSound>("qrc:/music/sky.wav")),
      mp_flySound(mkU<QSound>("qrc:/music/fly.wav")),
      mp_underwaterSound(mkU<QSound>("qrc:/music/underwater.wav")),
      mp_curBgmSound(mp_skySound.get()),
      mp_progShadow(mkU<ShaderProgram>(this)), mp_progDepth(mkU<ShaderProgram>(this)),
      m_shadowMappingFBO(-1), m_depthMap(-1),

      mp_playerStatusInspector(nullptr)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
    moveMouseToCenter();
    lastUpdateTime = QDateTime::currentMSecsSinceEpoch();

    for(bool &state: keyboardAllStars)
    {
        state = false;
    }

    updateCameraAlignedPosition();
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_worldAxes->destroy();
}


void MyGL::moveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::updateKeyboardControl()
{
    float scalingSpeed = 0.2f;
    float amount = 2.0f * scalingSpeed, normalizedAmount = amount, normalizedAmountWSAD = amount;

    if(keyboardAllStars[41]) // Shift
    {
        amount = 10.0f;
    }

    if(keyboardAllStars[37])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[39] && !keyboardAllStars[40]
                || !keyboardAllStars[39] && keyboardAllStars[40])
        {
            normalizedAmount *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->RotateAboutUp(normalizedAmount);
    }

    if(keyboardAllStars[38])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[39] && !keyboardAllStars[40]
                || !keyboardAllStars[39] && keyboardAllStars[40])
        {
            normalizedAmount *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->RotateAboutUp(-normalizedAmount);
    }

    if(keyboardAllStars[39])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[37] && !keyboardAllStars[38]
                || !keyboardAllStars[37] && keyboardAllStars[38])
        {
            normalizedAmount *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->RotateAboutRight(normalizedAmount);
    }

    if(keyboardAllStars[40])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[37] && !keyboardAllStars[38]
                || !keyboardAllStars[37] && keyboardAllStars[38])
        {
            normalizedAmount *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->RotateAboutRight(-normalizedAmount);
    }

    if(keyboardAllStars[Qt::Key_1])
    {
        mp_camera->fovy += amount;
    }

    if(keyboardAllStars[Qt::Key_2])
    {
        mp_camera->fovy -= amount;
    }

    if(keyboardAllStars[Qt::Key_W])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[Qt::Key_A] && !keyboardAllStars[Qt::Key_D]
                || !keyboardAllStars[Qt::Key_A] && keyboardAllStars[Qt::Key_D])
        {
            normalizedAmountWSAD *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->TranslateAlongLook(normalizedAmountWSAD);

    }

    if(keyboardAllStars[Qt::Key_S])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[Qt::Key_A] && !keyboardAllStars[Qt::Key_D]
                || !keyboardAllStars[Qt::Key_A] && keyboardAllStars[Qt::Key_D])
        {
            normalizedAmountWSAD *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->TranslateAlongLook(-normalizedAmountWSAD);
    }

    if(keyboardAllStars[Qt::Key_A])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[Qt::Key_W] && !keyboardAllStars[Qt::Key_S]
                || !keyboardAllStars[Qt::Key_W] && keyboardAllStars[Qt::Key_S])
        {
            normalizedAmountWSAD *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->TranslateAlongRight(-normalizedAmountWSAD);
    }

    if(keyboardAllStars[Qt::Key_D])
    {
        // if two keys are pressed together, damp the speed
        if(keyboardAllStars[Qt::Key_W] && !keyboardAllStars[Qt::Key_S]
                || !keyboardAllStars[Qt::Key_W] && keyboardAllStars[Qt::Key_S])
        {
            normalizedAmountWSAD *= 1.f / glm::sqrt(2.f);
        }
        mp_camera->TranslateAlongRight(normalizedAmountWSAD);
    }

    if(keyboardAllStars[Qt::Key_Q])
    {
        mp_camera->TranslateAlongUp(-amount);
    }

    if(keyboardAllStars[Qt::Key_E])
    {
        mp_camera->TranslateAlongUp(amount);
    }

    if(keyboardAllStars[Qt::Key_R])
    {
        *mp_camera = Camera(this->width(), this->height());
    }

    mp_camera->recomputeAttributes();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_POLYGON_SMOOTH);

    // For transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of Cube
    mp_worldAxes->create();

    mp_testCubeSet->create();
    mp_sketchCube->create();

    mp_lavaOverlay->create();
    mp_waterOverlay->create();

    mp_terrain->CreateTestScene();
    //mp_terrain->test1();
    mp_terrain->updateAllChunks();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the post-process shader
    mp_progLiquid->create(":/glsl/liquid.vert.glsl", ":/glsl/liquid.frag.glsl");
    // Create and set up shadow mapping shader
    mp_progShadow->create(":/glsl/shadow.vert.glsl", ":/glsl/shadow.frag.glsl");
    // Create and set up depth rendering shader
    mp_progDepth->create(":/glsl/depth.vert.glsl", ":/glsl/depth.frag.glsl");

//    mp_progPostprocess->create(":/glsl/passthrough.vert.glsl", ":/glsl/noOp.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    testCubeTexture->create(":/textures/minecraft_textures_all.png");
    testCubeTexture->load(0);

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
//    vao.bind();
    glGenFramebuffers(1, &m_shadowMappingFBO);
    glGenTextures(1, &m_depthMap);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMappingFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *mp_camera = Camera(w, h, glm::vec3(mp_terrain->dimensions.x * 0.5f, mp_terrain->dimensions.y - 1, mp_terrain->dimensions.z * 0.5f),
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));

    glm::mat4 viewproj = mp_camera->getViewProj();

    mp_terrain->offsetX = floor((mp_camera->eye.x - 32.0f) / 16.0f);
    mp_terrain->offsetZ = floor((mp_camera->eye.z - 32.0f) / 16.0f);
    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
//    mp_progPostprocess->setDimensions(glm::ivec2(w, h));
    printGLErrorLog();
}

// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    update();
    mp_player->updateDeltaTime(getElapsedTime());
    lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    mp_player->setCameraPtr(mp_camera.get());
    mp_player->updateVelocity();

    if (mp_playerStatusInspector == nullptr)
    {
        mp_playerStatusInspector = mkU<PlayerStatusInspector>(mp_player.get(), mp_terrain.get());
        mp_playerStatusInspector->start();
    }

    switch (mp_player->getMotionState())
    {
    case Motion::FLYING:
        mp_player->flyWithoutCollisions();
        if (mp_player->isWillingToMove() && mp_flySound->isFinished())
        {
            mp_flySound->play();
        }
        break;
    case Motion::WEIGHTLESSNESS:
        mp_player->freeFall(mp_terrain.get());
        break;
    case Motion::WALKING:
        mp_player->easyMoveWithCollisions(mp_terrain.get());
        if (mp_player->isWillingToMove() && mp_walkSound->isFinished())
        {
            mp_walkSound->play();
        }
        break;
    case Motion::SWIMMING:
        mp_player->easyMoveWithCollisions(mp_terrain.get());
        if (mp_player->isWillingToMove() && mp_swimSound->isFinished())
        {
            mp_swimSound->play();
        }
        break;
    default:
        break;
    }

    setBackgroundMusic();
    if (mp_curBgmSound->isFinished())
    {
        mp_curBgmSound->play();
    }
//    moveMouseToCenter();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());

//    mp_progPostprocess->setTime(lastUpdateTime);

    bool isUpdate = false;

    if(mp_terrain->offsetX != floor((mp_camera->eye.x - 32.0f) / 16.0f))
    {
        mp_terrain->offsetX = floor((mp_camera->eye.x - 32.0f) / 16.0f);
        isUpdate = true;
    }

    if(mp_terrain->offsetZ != floor((mp_camera->eye.z - 32.0f) / 16.0f))
    {
        mp_terrain->offsetZ = floor((mp_camera->eye.z - 32.0f) / 16.0f);
        isUpdate = true;
    }

    if(isUpdate)
    {
        for(int i = 0; i < 16; ++i)
        {
            TerrainCreator* terrainCreator = new TerrainCreator(&m_mutex, mp_terrain.get(), i);
            mp_threadPool->start(terrainCreator);
        }
        mp_threadPool->waitForDone();

        for(int i = 0; i < 16; ++i)
        {
            TerrainUpdater* terrainUpdater = new TerrainUpdater(&m_mutex, mp_terrain.get(), i);
            mp_threadPool->start(terrainUpdater);
        }
        mp_threadPool->waitForDone();

        for(int i = 0; i < 16; ++i)
        {
            mp_terrain->getChunkAt(i)->sendGPUDirectives();
        }
        //  mp_terrain->generateTerrain();
        //  mp_terrain->updateAllChunks();
    }

    // 1. Render shadow mapping texture
    glm::mat4 lightProjection, lightView;
    glm::vec3 lightPos = glm::vec3(64.0f + mp_terrain->offsetX * 16.0f, 300.0f, mp_terrain->offsetZ * 16.0f);
    lightProjection = glm::ortho(-60.0f, 60.0f, -60.0f, 60.0f, 1.0f, 300.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(64.0f + mp_terrain->offsetX * 16.0f, 256.0f, 64.0f + mp_terrain->offsetZ * 16.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    mp_progDepth->setLightMatrix(lightSpaceMatrix);
    GLDrawShadow();

    glViewport(0, 0, this->width(), this->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Use shadow map to render images
    mp_progLambert->setShadowTexture(1);
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setLightPos(lightPos);
    mp_progLambert->setLightMatrix(lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    GLDrawScene();

    if (mp_player->getCameraLocatedBlock(mp_terrain.get()) == BlockType::WATER)
    {
        mp_progLiquid->draw(*mp_waterOverlay);
    }
    else if (mp_player->getCameraLocatedBlock(mp_terrain.get()) == BlockType::LAVA)
    {
        mp_progLiquid->draw(*mp_lavaOverlay);
    }

    testCubeTexture->bind(0);
    GLDrawTestCubes();

    if(enableMovingCastle)
    {
        updateSketchCube();
    }

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progLambert->setViewVec(-mp_camera->look);
    mp_progFlat->draw(*mp_worldAxes);
    glEnable(GL_DEPTH_TEST);

    mp_progFlat->setTime(m_time);
    mp_progLambert->setTime(m_time);
    ++m_time;
}

void MyGL::GLDrawTestCubes()
{
    mp_progLambert->setModelMatrix(glm::mat4());
    mp_progLambert->setViewVec(-mp_camera->look);
    mp_progLambert->draw(*mp_testCubeSet);
}

void MyGL::updateSketchCube()
{
    updateCameraAlignedPosition();
    //cout<<glm::to_string(cameraAlignedPosition)<<endl;
    mp_sketchCube->destroy();
    mp_sketchCube->create();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progLambert->setViewVec(-mp_camera->look);
    mp_progFlat->draw(*mp_sketchCube);
    glEnable(GL_DEPTH_TEST);
}

void MyGL::updateCameraAlignedPosition()
{
    glm::vec3 eye = mp_camera->eye;
    float yaw = mp_camera->yaw;
    float pitch = mp_camera->pitch;
    eye.x = glm::floor(eye.x);
    eye.y = glm::floor(eye.y);
    eye.z = glm::floor(eye.z);
    float distance = 5;
    eye.x += int(distance * cos(yaw * M_PI / 180.f) * cos(pitch * M_PI / 180.f));
    eye.z += int(distance * sin(yaw * M_PI / 180.f) * cos(pitch * M_PI / 180.f));
    eye.y += int(distance * sin(pitch * M_PI / 180.f));
    cameraAlignedPosition = eye;
}

void MyGL::GLDrawScene()
{
    // draw every chunk
    for(int i = 0; i < 16; ++i)
    {
        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(),
                                                      glm::vec3(Terrain::getChunkIndexX(i) * 16.f + mp_terrain->offsetX * 16.0f,
                                                                0.f,
                                                                Terrain::getChunkIndexZ(i) * 16.f + mp_terrain->offsetZ * 16.0f)));
        mp_progLambert->setViewVec(-mp_camera->look);
        mp_progLambert->draw(*mp_terrain->getChunkAt(i));
    }
}

void MyGL::GLDrawShadow()
{
    // Use shadow shader to draw every chunk
    mp_progDepth->useMe();
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMappingFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Front face culling
    glCullFace(GL_FRONT);
    for(int i = 0; i < 16; ++i)
    {
        mp_progDepth->setModelMatrix(glm::translate(glm::mat4(),
                                                      glm::vec3(Terrain::getChunkIndexX(i) * 16.f + mp_terrain->offsetX * 16.0f,
                                                                0.f,
                                                                Terrain::getChunkIndexZ(i) * 16.f + mp_terrain->offsetZ * 16.0f)));
        mp_progDepth->draw(*mp_terrain->getChunkAt(i));
    }
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
}


void MyGL::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        QCoreApplication::quit(); // to quit the program
    }

    mp_player->listenKeyPressEvent(e, true);
    if (e->key() == Qt::Key_PageDown)
    {
        lockMouse = !lockMouse;
    }
    if (e->key() <= Qt::Key_9 && e->key() >= Qt::Key_3
            || e->key() == Qt::Key_0
            || e->key() == Qt::Key_P)
    {
        updateCameraAlignedPosition();
        bool add = true;
        for(glm::vec3 &p: mp_testCubeSet->testCubePositions)
        {
            if(p == cameraAlignedPosition)
            {
                add = false;
            }
        }
        if (add)
        {
            testCubePositions.push_back(cameraAlignedPosition);
            switch (e->key())
            {
            case Qt::Key_5:
                testCubeTypes.push_back(GRASS);
                break;
            case Qt::Key_6:
                testCubeTypes.push_back(STONE);
                break;
            case Qt::Key_7:
                testCubeTypes.push_back(DIRT);
                break;
            case Qt::Key_8:
                testCubeTypes.push_back(WATER);
                break;
            case Qt::Key_9:
                testCubeTypes.push_back(LAVA);
                break;
            case Qt::Key_0:
                testCubeTypes.push_back(ICE);
                break;
            case Qt::Key_3:
                testCubeTypes.push_back(LEAF);
                break;
            case Qt::Key_4:
                testCubeTypes.push_back(BEDROCK);
                break;
            case Qt::Key_P:
                testCubeTypes.push_back(WOOD);
                break;
            }
        }
        mp_testCubeSet->destroy();
        mp_testCubeSet->create();
    }
    if(e->key() == Qt::Key_Plus)
    {
        enableMovingCastle = !enableMovingCastle;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent* e)
{
    mp_player->listenKeyPressEvent(e, false);
}

void MyGL::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        glm::vec3 deleteCube = mp_terrain->findNearestBlock(mp_camera->eye, mp_camera->look);
        if(!fequal(deleteCube.x, -0.1f))
        {
            mp_terrain->setBlockAt(deleteCube.x - mp_terrain->offsetX * 16.0f,
                                   deleteCube.y,
                                   deleteCube.z - mp_terrain->offsetZ * 16.0f, EMPTY);
            mp_terrain->playerChange(deleteCube, EMPTY);
            mp_terrain->updateAllChunks();
        }
    }

    if(event->button() == Qt::RightButton)
    {
        glm::vec3 newCube = mp_terrain->findNearestFace(mp_camera->eye, mp_camera->look);
        if(!fequal(newCube.x, -0.1f))
        {
            if(mp_terrain->getBlockAt(newCube.x - mp_terrain->offsetX * 16.0f,
                                      newCube.y,
                                      newCube.z - mp_terrain->offsetZ * 16.0f) == EMPTY)
            {
                mp_terrain->setBlockAt(newCube.x - mp_terrain->offsetX * 16.0f,
                                       newCube.y,
                                       newCube.z - mp_terrain->offsetZ * 16.0f, STONE);
                mp_terrain->playerChange(newCube, STONE);
                mp_terrain->updateAllChunks();
            }

        }
    }
}


void MyGL::mouseMoveEvent(QMouseEvent* e)
{
    mp_player->listenMouseEvent(e, MouseState::MOVED);
    moveMouseToCenter();
}

void MyGL::leaveEvent(QEvent* e)
{
    moveMouseToCenter();
}

int64_t MyGL::getElapsedTime()
{
    return QDateTime::currentMSecsSinceEpoch() - lastUpdateTime;
}

void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

//    mp_progPostprocess->draw(*mp_worldAxes);
}

//void MyGL::createRenderBuffers()
//{
//    // Initialize the frame buffers and render textures
//    glGenFramebuffers(1, &m_frameBuffer);
//    glGenTextures(1, &m_renderedTexture);
//    glGenRenderbuffers(1, &m_depthRenderBuffer);

//    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
//    // Bind our texture so that all functions that deal with textures will interact with this one
//    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
//    // Give an empty image to OpenGL ( the last "0" )
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);

//    // Set the render settings for the texture we've just created.
//    // Essentially zero filtering on the "texture" so it appears exactly as rendered
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    // Clamp the colors at the edge of our texture
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//    // Initialize our depth buffer
//    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

//    // Set m_renderedTexture as the color output of our frame buffer
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

//    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
//    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
//    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

//    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//    {
//        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
//        printGLErrorLog();
//    }
//}

void MyGL::setBackgroundMusic()
{
    if (mp_player->getMotionState() == Motion::FLYING)
    {
        if (mp_curBgmSound != mp_skySound.get())
        {
            mp_curBgmSound->stop();
            mp_curBgmSound = mp_skySound.get();
        }
        return;
    }

    if (mp_player->isSubmergedInLiquid(mp_terrain.get()))
    {
        if (mp_curBgmSound != mp_underwaterSound.get())
        {
            mp_curBgmSound->stop();
            mp_curBgmSound = mp_underwaterSound.get();
        }
        return;
    }

    if (mp_curBgmSound != mp_bgmSound.get())
    {
        mp_curBgmSound->stop();
        mp_curBgmSound = mp_bgmSound.get();
    }
}
