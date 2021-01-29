#pragma once
#include <QList>
#include <la.h>
#include <exception>
#include <drawable.h>
#include <array>
#include <smartpointerhelp.h>
#include <map>
#include <tuple>
#include "rivergenerator.h"
#include "buildinggenerator.h"
#include <unordered_map>
#include <bitset>
#include <texture.h>
#include <QMutex>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY,
    GRASS,
    DIRT,
    STONE,
    LAVA,
    WOOD,
    LEAF,
    BEDROCK,
    WATER,
    ICE
};

bool isAnimated(BlockType t);

bool isTransparent(BlockType t);
bool isAnimated(int t);

bool isTransparent(int t);


class Terrain;

class Chunk: public Drawable
{
    friend class Terrain;

private:
    // linearly arranged chunks
    std::array<BlockType, 65536> m_blocks;

    // the owner of this chunk
    Terrain *master;
    int chunkOffset;

    std::vector<glm::vec4> vertexBufferData;
    std::vector<GLuint> indexBufferData;

    // set colors
    const glm::vec4 colorDirt = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f,
            colorGrass = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f,
            colorStone = glm::vec4(0.5f, 0.5f, 0.5f, 1.f),
            colorLava = glm::vec4(1.0f, 0.0f, 0.0f, 1.f),
            colorWater = glm::vec4(0.0f, 0.0f, 1.0f, 0.3f);

public:
    Chunk(OpenGLContext* context, Terrain *terrain = nullptr, int offset = 0);

    // inherited from Drawable
    virtual ~Chunk() {}
    void create();
    GLenum drawMode();

    void sendGPUDirectives();
    void sendGPUDirectives(QMutex* mutex);

    // process a cube
    void addCubeData(int cubeIndex, std::vector<glm::vec4> &vertexBufferData, std::vector<GLuint> &indexBufferData);

    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.

    BlockType &getBlockAt(unsigned int x, unsigned int y, unsigned int z);

    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.

    BlockType operator()(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType &operator()(unsigned int x, unsigned int y, unsigned int z);

    // static functions for coordinate mapping
    static unsigned int getBlockIndexX(int i) { return (i & 255) >> 4; }
    static unsigned int getBlockIndexY(int i) { return i >> 8; }
    static unsigned int getBlockIndexZ(int i) { return i & 15; }
    static unsigned int getBlockIndex(int x, int y, int z) { return (y << 8) + (x << 4) + z; }
};

class Terrain
{
    friend class Chunk;

private:
    std::array<uPtr<Chunk>, 16> chunks;

    // Players' operation
    std::map<std::tuple<float, float, float>, BlockType> playerOp;
    RiverGenerator riverGen;
    BuildingGenerator buildingGen;

public:
    // Current offset
    int offsetX = 0;
    int offsetZ = 0;

    Terrain(OpenGLContext* context);

    Chunk *getChunkAt(int i);

    // destroy and create for rendering
    void updateAllChunks();
    void updateChunkAt(int index);

    void CreateTestScene();

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    BlockType &getBlockAt(int x, int y, int z);

    BlockType operator()(int x, int y, int z) const;
    BlockType &operator()(int x, int y, int z);

    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
    float fbm(glm::vec2 n);

    float noise2D(glm::vec2 n);

    void generateTerrain();

    void generateTerrainAt(int index);

    float perlinNoise(glm::vec2 n);

    glm::vec2 randomVector(glm::vec2 n);

    // static functions for coordinate mapping
    static unsigned int getChunkIndexX(int i) { return i >> 2; } // Get x from linear chunk index
    static unsigned int getChunkIndexZ(int i) { return i & 3; } // Get z from linear chunk index
    static unsigned int getChunkIndex(int x, int z) { return (x << 2) + z; } // get linear chunk index from x and z

    static glm::ivec2 getChunkBlockIndexFromBlockIndex(int x, int y, int z)
    {
        return glm::ivec2(getChunkIndex(x >> 4, z >> 4), Chunk::getBlockIndex(x & 15, y, z & 15));
    }

    static glm::ivec3 getBlockIndexXYZFromChunkBlockIndex(int chunkIndex, int chunkBlockIndex);


    glm::vec3 findNearestBlock(glm::vec3& playerPos, glm::vec3& eyeSight); // Function for remove block

    glm::vec3 findNearestFace(glm::vec3& playerPos, glm::vec3& eyeSight);  // Function for place a block

    void updateChunkForUpdatedBlock(int x, int y, int z);

    // Player change
    void playerChange(glm::vec3, BlockType b);

};

glm::vec4 clampColor(const glm::vec4 &color);

class TestCubeSet: public Drawable
{
public:
    TestCubeSet(OpenGLContext* context,
                std::vector<glm::vec3> &_testCubePositions,
                std::vector<int> &_testCubeTypes)
        : Drawable(context), testCubePositions(_testCubePositions), testCubeTypes(_testCubeTypes) {}
    virtual ~TestCubeSet(){}
    void create() override;

    std::vector<glm::vec3> &testCubePositions;
    std::vector<int> &testCubeTypes;

    void addCubeData(int cubeIndex, std::vector<glm::vec4> &vertexBufferData, std::vector<GLuint> &indexBufferData);


    // set colors
    const glm::vec4 colorDirt = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f,
            colorGrass = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f,
            colorStone = glm::vec4(0.5f, 0.5f, 0.5f, 1.f),
            colorLava = glm::vec4(1.f, 0.0f, 0.0f, 1.f);
};

class SketchCube: public Drawable
{
public:
    SketchCube(OpenGLContext* context, glm::vec3 &_position)
        : Drawable(context), position(_position) {}
    virtual ~SketchCube(){}
    void create() override;
    GLenum drawMode() override;
    glm::vec3 &position;
};
