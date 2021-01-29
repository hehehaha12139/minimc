#include <scene/terrain.h>
#include <iostream>
#include <scene/cube.h>
#include <vector>
#include <utils.h>
#include <QTime>

Terrain::Terrain(OpenGLContext* context): dimensions(64, 256, 64)
{
    // initialize every chunk
    for(int i = 0; i < chunks.size(); ++i)
    {
        chunks[i] = mkU<Chunk>(context, this, i);
    }

    // River generation
    riverGen = RiverGenerator(glm::vec2(0.0f, 0.0f), glm::normalize(glm::vec2(1.0f, 1.0f)));
    buildingGen = BuildingGenerator(glm::vec2(16.0f, -16.0f), glm::normalize(glm::vec2(0.0f, 1.0f)));
    std::string rule = "FLR";
    std::string buildingRule = "M";
    riverGen.generate(riverGen.expansion(rule, 4));
    buildingGen.generate(buildingGen.expansion(buildingRule, 3));
}

Chunk *Terrain::getChunkAt(int i)
{
    return chunks[i].get();
}

// destroy and create for rendering
void Terrain::updateAllChunks()
{
    for(uPtr<Chunk> &pChunk: chunks)
    {
        pChunk->destroy();
        pChunk->create();
        pChunk->sendGPUDirectives();
    }
}

void Terrain::updateChunkAt(int index)
{
    chunks[index]->destroy();
    chunks[index]->create();
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    glm::ivec2 block = getChunkBlockIndexFromBlockIndex(x, y, z);
    return chunks[block[0]]->m_blocks[block[1]];
}

BlockType &Terrain::getBlockAt(int x, int y, int z)
{
    glm::ivec2 block = getChunkBlockIndexFromBlockIndex(x, y, z);
    return chunks[block[0]]->m_blocks[block[1]];
}

BlockType Terrain::operator()(int x, int y, int z) const
{
    return getBlockAt(x, y, z);
}

BlockType &Terrain::operator()(int x, int y, int z)
{
    return getBlockAt(x, y, z);
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    (*this)(x, y, z) = t;
}

void Terrain::CreateTestScene()
{
    generateTerrain();
}

float Terrain::fbm(glm::vec2 n)
{
    float result = 0.0f;
    float persistence = 0.5f;
    int octaves = 16;

    for(int i = 1; i <= octaves; i++)
    {
        float freq = pow(2.0f, i);
        float amp = pow(persistence, i);

        float x = n.x * freq;
        float y = n.y * freq;

        float v1 = noise2D(glm::vec2(floor(x), floor(y)));
        float v2 = noise2D(glm::vec2(floor(x) + 1, floor(y)));
        float v3 = noise2D(glm::vec2(floor(x), floor(y) + 1));
        float v4 = noise2D(glm::vec2(floor(x) + 1, floor(y) + 1));

        float i1 = glm::mix(v1, v2, glm::fract(x));
        float i2 = glm::mix(v3, v4, glm::fract(x));

        result += glm::mix(i1, i2, glm::fract(y)) * amp;
    }

    return pow(result, 1.0f);
}

float Terrain::noise2D(glm::vec2 n)
{
     return glm::fract(
                 std::sin(
                     glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453);
}

void Terrain::generateTerrain()
{
    for(int x = 0; x < 64; x++)
    {
        for(int z = 0; z < 64; z++)
        {
            bool isRiver = false;
            bool isBuilding = false;
            int riverDepth = 0;
            // Detect waterBlock
            long long int waterIndex = ((long long int)(x + offsetX * 16) << 32) + (long long int)(z + offsetZ * 16);
//            if(riverGen.waterCubes.find(waterIndex) != riverGen.waterCubes.end())
//            {
//                isRiver = true;
//                riverDepth = riverGen.waterCubes[waterIndex];
//            }

            // Detect buildingBlock
            long long int buildingIndex = ((long long int)(x + offsetX * 16) << 32) + (long long int)(z + offsetZ * 16);
            if(buildingGen.buildingBlock.find(buildingIndex) != buildingGen.buildingBlock.end())
            {
                isBuilding = true;
            }

            int height = floor(pow(fbm(glm::vec2((x + offsetX * 16)/ 64.0f, (z + offsetZ * 16)/ 64.0f)), 0.1) * 256);

            int normalHeight = height;

            if(isRiver)
            {
//                    if(riverDepth < 0)
//                    {
//                        height = floor(((float)height - 220) * (float(abs(riverDepth)) / 5) + 220);
//                        std::cout<<std::endl;
//                    }
//                    else
//                    {
//                        height = 220;
//                        normalHeight = height - riverDepth;
//                    }
                height = 220;
                if(riverDepth < 0)
                {
                    int river = floor((float)(normalHeight - height) * (float)riverDepth / 12.0f);
                    normalHeight = height - river;
                }
                else
                {
                    normalHeight = height - riverDepth;
                }
            }

            if(isBuilding)
            {
                height = 180;
                // Set a plane for building
                for(int y = 0; y < 256; y++)
                {
                    std::tuple<float, float, float> posTuple = std::make_tuple(float(x + offsetX * 16), float(y), float(z + offsetZ * 16));
                    if(playerOp.find(posTuple) != playerOp.end())
                    {
                       setBlockAt(x, y, z, playerOp[posTuple]);
                       continue;
                    }

                    if(y <= 128)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else if(y < height)
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                    else
                    {
                        setBlockAt(x, y, z, EMPTY);
                    }
                }
            }
            else
            {
                // Set all block
                for(int y = 0; y < 256; y++)
                {
                    std::tuple<float, float, float> posTuple = std::make_tuple(float(x + offsetX * 16), float(y), float(z + offsetZ * 16));
                    if(playerOp.find(posTuple) != playerOp.end())
                    {
                       setBlockAt(x, y, z, playerOp[posTuple]);
                       continue;
                    }

                    if(y <= 128)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else if(y < normalHeight - 1)
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                    else if(y < height)
                    {
                        setBlockAt(x, y, z, WATER);
                    }
                    else
                    {
                        setBlockAt(x, y, z, EMPTY);
                    }

                    if(y == normalHeight - 1 && (!isRiver || riverDepth < 0))
                    {
                        setBlockAt(x, y, z, GRASS);
                    }
                }
            }
        }
    }
}

void Terrain::generateTerrainAt(int index)
{
    for(int x = index * 4; x < (index + 1) * 4; ++x)
    {
        for(int z = 0; z < 64; z++)
        {
            bool isRiver = false;
            bool isBuilding = false;
            int buildFlag = -5;
            int riverDepth = 0;
            // Detect waterBlock
            long long int waterIndex = ((long long int)(x + offsetX * 16) << 32) + (long long int)(z + offsetZ * 16);
//            if(riverGen.waterCubes.find(waterIndex) != riverGen.waterCubes.end())
//            {
//                isRiver = true;
//                riverDepth = riverGen.waterCubes[waterIndex];
//            }

            // Detect buildingBlock
            long long int buildingIndex = ((long long int)(x + offsetX * 16) << 32) + (long long int)(z + offsetZ * 16);
            if(buildingGen.buildingBlock.find(buildingIndex) != buildingGen.buildingBlock.end())
            {
                isBuilding = true;
                buildFlag = buildingGen.buildingBlock[buildingIndex];
            }

            int height = floor(pow(fbm(glm::vec2((x + offsetX * 16)/ 64.0f, (z + offsetZ * 16)/ 64.0f)), 0.1) * 256);

            int normalHeight = height;

            if(isRiver)
            {
                height = 220;
                if(riverDepth < 0)
                {
                    int river = floor((float)(normalHeight - height) * (float)riverDepth / 12.0f);
                    normalHeight = height - river;
                }
                else
                {
                    normalHeight = height - riverDepth;
                }
            }

            if(isBuilding)
            {
                height = 180;
                // Set a plane for building
                for(int y = 0; y < 256; y++)
                {
                    int buildHeight = height;
                    if(buildFlag != -5)
                    {
                        buildHeight = 200;
                    }
                    std::tuple<float, float, float> posTuple = std::make_tuple(float(x + offsetX * 16), float(y), float(z + offsetZ * 16));
                    if(playerOp.find(posTuple) != playerOp.end())
                    {
                       setBlockAt(x, y, z, playerOp[posTuple]);
                       continue;
                    }

                    if(y <= 128)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else if(y < height)
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                    else if(y < buildHeight)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else
                    {
                        setBlockAt(x, y, z, EMPTY);
                    }
                }
            }
            else
            {
                // Set all block
                for(int y = 0; y < 256; y++)
                {
                    std::tuple<float, float, float> posTuple = std::make_tuple(float(x + offsetX * 16), float(y), float(z + offsetZ * 16));
                    if(playerOp.find(posTuple) != playerOp.end())
                    {
                       setBlockAt(x, y, z, playerOp[posTuple]);
                       continue;
                    }

                    if(y <= 128)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else if(y < normalHeight - 1)
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                    else if(y < height)
                    {
                        setBlockAt(x, y, z, WATER);
                    }
                    else
                    {
                        setBlockAt(x, y, z, EMPTY);
                    }

                    if(y == normalHeight - 1 && (!isRiver || riverDepth < 0))
                    {
                        setBlockAt(x, y, z, GRASS);
                    }
                }
            }
        }
    }
}

float Terrain::perlinNoise(glm::vec2 n)
{
    int gridXIndex = n.x / 8;
    int gridYIndex = n.y / 8;

    glm::vec2 nPos = glm::vec2(gridXIndex + (int(n.x) % 8) / 8.0f, gridYIndex + (int(n.y) % 8) / 8.0f);

    std::vector<glm::vec2> vertices;
    glm::vec2 gridLeftDown = glm::vec2(gridXIndex, gridYIndex);
    glm::vec2 gridRightDown = glm::vec2(gridXIndex + 1, gridYIndex);
    glm::vec2 gridLeftUp = glm::vec2(gridXIndex, gridYIndex + 1);
    glm::vec2 gridRightUp = glm::vec2(gridXIndex + 1, gridXIndex + 1);
    vertices.push_back(gridLeftDown);
    vertices.push_back(gridRightDown);
    vertices.push_back(gridLeftUp);
    vertices.push_back(gridRightUp);

    // Vector from vertices to given point
    std::vector<glm::vec2> verticesVector;
    for(int i = 0; i < 4; i++)
    {
        verticesVector.push_back(glm::normalize(nPos - vertices[i]));
    }

    // Random vectors
    std::vector<glm::vec2> randomVec;
    for(int i = 0; i < 4; i++)
    {
        glm::vec2 randomPos = randomVector(verticesVector[i]);
        randomVec.push_back(glm::normalize(randomPos - vertices[i]));
    }

    float result = 0.0f;
    float total =  0.0f;
    for(int i = 0; i < 4; i++)
    {
        float length = glm::length(nPos - vertices[i]);
        float coef = length;
        total += length;
        coef = coef * coef * coef * (coef * (coef * 6 - 15) + 10);
        result += coef * glm::dot(randomVec[i], verticesVector[i]);
    }

    return result / total;
}

glm::vec2 Terrain::randomVector(glm::vec2 n)
{
    return glm::fract(glm::vec2(sin(glm::dot(n,glm::vec2(127.1, 311.7))),
                                sin(glm::dot(n,glm::vec2(269.5, 183.3)))) * 43758.5453f);
}

glm::ivec3 Terrain::getBlockIndexXYZFromChunkBlockIndex(int chunkIndex, int chunkBlockIndex)
{
    // transform index from chunk field to terrain field
    unsigned int x = (getChunkIndexX(chunkIndex) << 4) + Chunk::getBlockIndexX(chunkBlockIndex);
    unsigned int z = (getChunkIndexZ(chunkIndex) << 4) + Chunk::getBlockIndexZ(chunkBlockIndex);
    unsigned int y = Chunk::getBlockIndexY(chunkBlockIndex);
    if(x >= 64 || y >= 256 || z >= 64)
    {
        throw std::out_of_range("Subscript out of range");
    }
    return glm::ivec3(x, y, z);
}

Chunk::Chunk(OpenGLContext *context, Terrain *terrain, int offset): Drawable(context), master(terrain), chunkOffset(offset)
{
    for(BlockType &block: m_blocks)
    {
        block = EMPTY;
    }
}

void Chunk::create()
{
    vertexBufferData.clear();
    indexBufferData.clear();

    // interate over every block
    for(int i = 0; i < 65536; ++i)
    {
        if (m_blocks[i] == EMPTY)
        {
            continue;
        }
        if(!isTransparent(m_blocks[i]))
        {
            addCubeData(i, vertexBufferData, indexBufferData);
        }
    }
    for(int i = 0; i < 65536; ++i)
    {
        if (m_blocks[i] == EMPTY)
        {
            continue;
        }
        if(isTransparent(m_blocks[i]))
        {
            addCubeData(i, vertexBufferData, indexBufferData);
        }
    }

    count = indexBufferData.size();
}

void Chunk::sendGPUDirectives()
{
    if(indexBufferData.empty())
    {
        return;
    }

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec4), vertexBufferData.data(), GL_STATIC_DRAW);
}

void Chunk::sendGPUDirectives(QMutex* mutex)
{
    if(indexBufferData.empty())
    {
        return;
    }

    mutex->lock();
    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec4), vertexBufferData.data(), GL_STATIC_DRAW);
    mutex->unlock();

}

GLenum Chunk::drawMode()
{
    return GL_TRIANGLES;
}

// process a cube
void Chunk::addCubeData(int cubeIndex, std::vector<glm::vec4> &vertexBufferData, std::vector<GLuint> &indexBufferData)
{
    glm::vec4 color(0, 0, 0, 1);;
    switch(m_blocks[cubeIndex])
    {
    case DIRT:
        color = colorDirt;
        break;
    case GRASS:
        color = colorGrass;
        break;
    case STONE:
        color = colorStone;
        break;
    case LAVA:
        color = colorLava;
        break;
    case WATER:
        color = colorWater;
        break;
    default:
        break;
    }
    glm::vec4 positionOffset((float)getBlockIndexX(cubeIndex),
                             (float)getBlockIndexY(cubeIndex),
                             (float)getBlockIndexZ(cubeIndex), 0.f);

    glm::vec2 uvFront(0, 0), uvBack(0, 0), uvLeft(0, 0), uvRight(0, 0), uvTop(0, 0), uvBottom(0, 0);

    float _blinnPower = 0, _isAnimated = 0;

    int blockType = m_blocks[cubeIndex];

    if(isAnimated((BlockType)blockType))
    {
        _isAnimated = 1;
    }

    switch(blockType)
    {
    case DIRT:
        _blinnPower = 0;
        color = colorDirt;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(2, 0);
        break;
    case GRASS:
        _blinnPower = 0;
        color = colorGrass;
        if(cubeIndex + 256 < m_blocks.size() && m_blocks[cubeIndex + 256] == GRASS)
        {
            uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(2, 0);
        }
        else
        {
            uvFront = uvBack = uvLeft = uvRight = glm::vec2(3, 0);
            uvTop = glm::vec2(8, 2);
            uvBottom = glm::vec2(2, 0);
        }
        break;
    case STONE:
        _blinnPower = 10;
        color = colorStone;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(1, 0);
        break;
    case LAVA:
        _blinnPower = 30;
        color = colorLava;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(14, 15);
        break;
    case WATER:
        _blinnPower = 50;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(14, 13);
        break;
    case ICE:
        _blinnPower = 30;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(3, 4);
        break;
    case BEDROCK:
        _blinnPower = 10;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(1, 1);
        break;
    case LEAF:
        _blinnPower = 5;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(5, 3);
        break;
    case WOOD:
        _blinnPower = 5;
        uvFront = uvBack = uvLeft = uvRight = glm::vec2(4, 1);
        uvTop = uvBottom = glm::vec2(5, 1);
        break;
    default:
        break;
    }

    // add data for a face to be rendered
    auto addFace = [&vertexBufferData, &indexBufferData, &positionOffset]
            (const glm::vec4 &position_1, const glm::vec4 &position_2, const glm::vec4 &position_3, const glm::vec4 &position_4,
                const glm::vec4 &normal, const glm::vec4 &color, const glm::vec2 &uv, float _blinnPower, float _isAnimated)
    {
        unsigned int startingIndex = vertexBufferData.size() / 4;
        std::array<glm::vec4, 4> additionalInfo;

        // UV info
        additionalInfo[0][0] = uv.x + 1, additionalInfo[0][1] = 16 - uv.y;
        additionalInfo[1][0] = uv.x + 1, additionalInfo[1][1] = 16 - uv.y - 1;
        additionalInfo[2][0] = uv.x, additionalInfo[2][1] = 16 - uv.y - 1;
        additionalInfo[3][0] = uv.x, additionalInfo[3][1] = 16 - uv.y;

        for(int i = 0; i < 4; ++i)
        {
            additionalInfo[i][0] *= 0.0625f;
            additionalInfo[i][1] *= 0.0625f;
        }

        // blinn power
        additionalInfo[0][2] = _blinnPower;
        additionalInfo[1][2] = _blinnPower;
        additionalInfo[2][2] = _blinnPower;
        additionalInfo[3][2] = _blinnPower;

        // is animated
        additionalInfo[0][3] = _isAnimated;
        additionalInfo[1][3] = _isAnimated;
        additionalInfo[2][3] = _isAnimated;
        additionalInfo[3][3] = _isAnimated;

        indexBufferData.insert(indexBufferData.end(),
            { startingIndex, startingIndex + 1, startingIndex + 2,
              startingIndex, startingIndex + 2, startingIndex + 3 });
        vertexBufferData.insert(vertexBufferData.end(),
            { positionOffset + position_1, normal, color, additionalInfo[0],
              positionOffset + position_2, normal, color, additionalInfo[1],
              positionOffset + position_3, normal, color, additionalInfo[2],
              positionOffset + position_4, normal, color, additionalInfo[3] });
    };

    float offset1 = 0.f, offset2 = 0.f, offset3 = 0.f;
    glm::vec4 colorFront = glm::vec4{color.x, color.y, color.z, color.w};
    glm::vec4 colorBack = glm::vec4{color.x + offset1, color.y + offset1, color.z + offset1, color.w};
    glm::vec4 colorLeft = glm::vec4{color.x - offset1, color.y - offset1, color.z - offset1, color.w};
    glm::vec4 colorRight = glm::vec4{color.x + offset2, color.y + offset2, color.z + offset2, color.w};
    glm::vec4 colorTop = glm::vec4{color.x - offset2, color.y - offset2, color.z - offset2, color.w};
    glm::vec4 colorBottom = glm::vec4{color.x + offset3, color.y + offset3, color.z + offset3, color.w};
    colorFront = clampColor(colorFront);
    colorBack = clampColor(colorBack);
    colorLeft = clampColor(colorLeft);
    colorRight = clampColor(colorRight);
    colorTop = clampColor(colorTop);
    colorBottom = clampColor(colorBottom);

    auto addFront = [addFace, colorFront, uvFront, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
                glm::vec4(0.f, 0.f, 1.f, 0.f), colorFront, uvFront, _blinnPower, _isAnimated);
    };

    auto addBack = [addFace, colorBack, uvBack, _blinnPower, _isAnimated] {
        addFace(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(0.f, 0.f, -1.f, 0.f), colorBack, uvBack, _blinnPower, _isAnimated);
    };

    auto addTop = [addFace, colorTop, uvTop, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(0.f, 1.f, 0.f, 0.f), colorTop, uvTop, _blinnPower, _isAnimated);
    };

    auto addBottom = [addFace, colorBottom, uvBottom, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                glm::vec4(0.f, -1.f, 0.f, 0.f), colorBottom, uvBottom, _blinnPower, _isAnimated);
    };

    auto addLeft = [addFace, colorLeft, uvLeft, _blinnPower, _isAnimated] {
        addFace(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(-1.f, 0.f, 0.f, 0.f), colorLeft, uvLeft, _blinnPower, _isAnimated);
    };

    auto addRight = [addFace, colorRight, uvRight, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                glm::vec4(1.f, 0.f, 0.f, 0.f), colorRight, uvRight, _blinnPower, _isAnimated);
    };

    std::bitset<6> visibility("111111");

    bool frontEnabled = visibility[0];
    bool backEnabled = visibility[1];
    bool leftEnabled = visibility[2];
    bool rightEnabled = visibility[3];
    bool topEnabled = visibility[4];
    bool bottomEnabled = visibility[5];

    BlockType testedNeighbor;
    auto shouldAdd = [](int blockType, BlockType testedNeighbor) -> bool
    {
        return testedNeighbor == EMPTY || !isTransparent(blockType) && isTransparent(testedNeighbor);
    };

    // Front
    if (frontEnabled)
    {
        if (positionOffset.z != 15)
        {
            testedNeighbor = m_blocks[cubeIndex + 1];
            if (shouldAdd(blockType, testedNeighbor))
            {
                addFront();
            }
        }
        else
        {
            if (master)
            {
                if (master->getChunkIndexZ(chunkOffset) == 3)
                {
                    addFront();
                }
                else
                {
                    glm::vec3 terrainIndex(master->getBlockIndexXYZFromChunkBlockIndex(chunkOffset, cubeIndex));
                    testedNeighbor = (*master)(terrainIndex.x, terrainIndex.y, terrainIndex.z + 1);
                    if (shouldAdd(blockType, testedNeighbor))
                    {
                        addFront();
                    }
                }
            }
            else
            {
                addFront();
            }
        }
    }

    // Back
    if (backEnabled)
    {
        if (positionOffset.z != 0)
        {
            testedNeighbor = m_blocks[cubeIndex - 1];
            if(shouldAdd(blockType, testedNeighbor))
            {
                addBack();
            }
        }
        else
        {
            if (master)
            {
                if (master->getChunkIndexZ(chunkOffset) == 0)
                {
                    addBack();
                }
                else
                {
                    glm::vec3 terrainIndex(master->getBlockIndexXYZFromChunkBlockIndex(chunkOffset, cubeIndex));
                    testedNeighbor = (*master)(terrainIndex.x, terrainIndex.y, terrainIndex.z - 1);
                    if(shouldAdd(blockType, testedNeighbor))
                    {
                        addBack();
                    }
                }
            }
            else
            {
                addBack();
            }
        }
    }

    // Right
    if (rightEnabled)
    {
        if (positionOffset.x != 15)
        {
            testedNeighbor = m_blocks[cubeIndex + 16];
            if(shouldAdd(blockType, testedNeighbor))
            {
                addRight();
            }
        }
        else
        {
            if (master)
            {
                if (master->getChunkIndexX(chunkOffset) == 3)
                {
                    addRight();
                }
                else
                {
                    glm::vec3 terrainIndex(master->getBlockIndexXYZFromChunkBlockIndex(chunkOffset, cubeIndex));
                    testedNeighbor = (*master)((int)terrainIndex.x + 1, (int)terrainIndex.y, (int)terrainIndex.z);
                    if(shouldAdd(blockType, testedNeighbor))
                    {
                        addRight();
                    }
                }
            }
            else
            {
                addRight();
            }
        }
    }

    // Left
    if (leftEnabled)
    {
        if (positionOffset.x != 0)
        {
            testedNeighbor = m_blocks[cubeIndex - 16];
            if(shouldAdd(blockType, testedNeighbor))
            {
                addLeft();
            }
        }
        else
        {
            if (master)
            {
                if (master->getChunkIndexX(chunkOffset) == 0)
                {
                    addLeft();
                }
                else
                {
                    glm::vec3 terrainIndex(master->getBlockIndexXYZFromChunkBlockIndex(chunkOffset, cubeIndex));
                    testedNeighbor = (*master)(terrainIndex.x - 1, terrainIndex.y, terrainIndex.z);
                    if(shouldAdd(blockType, testedNeighbor))
                    {
                        addLeft();
                    }
                }
            }
            else
            {
                addLeft();
            }
        }
    }

    // Top
    if (topEnabled)
    {
        if (positionOffset.y != 255)
        {
            testedNeighbor = m_blocks[cubeIndex + 256];
            if(shouldAdd(blockType, testedNeighbor))
            {
                addTop();
            }
        }
        else
        {
            addTop();
        }
    }

    // Bottom
    if (bottomEnabled)
    {
        if (positionOffset.y != 0)
        {
            testedNeighbor = m_blocks[cubeIndex - 256];
            if(shouldAdd(blockType, testedNeighbor))
            {
                addBottom();
            }
        }
        else
        {
            addBottom();
        }
    }
}

BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const
{
    if (x >= 16 || y >= 256 || z >= 16)
    {
        throw std::out_of_range("Subscript out of range");
    }
    return m_blocks[(y << 8) + (x << 4) + z];
}

BlockType &Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z)
{
    if (x >= 16 || y >= 256 || z >= 16)
    {
        throw std::out_of_range("Subscript out of range");
    }
    return m_blocks[(y << 8) + (x << 4) + z];
}

void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t)
{
    if (x >= 16 || y >= 256 || z >= 16)
    {
        throw std::out_of_range("Subscript out of range");
    }
    m_blocks[(y << 8) + (x << 4) + z] = t;
}

BlockType Chunk::operator()(unsigned int x, unsigned int y, unsigned int z) const
{
    return getBlockAt(x, y, z);
}

BlockType &Chunk::operator()(unsigned int x, unsigned int y, unsigned int z)
{
    return getBlockAt(x, y, z);
}

glm::vec3 Terrain::findNearestBlock(glm::vec3& playerPos, glm::vec3& eyeSight)
{
    glm::vec3 minBlock = glm::vec3(-0.1f, -0.1f, -0.1f);
    glm::vec3 eyeDetect;
    for(float t = 0.2f; !fequal(t, 4.0f); t = t + 0.2f)
    {
        eyeDetect = playerPos + t * eyeSight;

        if(getBlockAt(floor(eyeDetect.x) - offsetX * 16, floor(eyeDetect.y), floor(eyeDetect.z) - offsetZ * 16) != EMPTY)
        {
            minBlock = glm::vec3(floor(eyeDetect.x), floor(eyeDetect.y), floor(eyeDetect.z));
            break;
        }
    }

    return minBlock;
}

glm::vec3 Terrain::findNearestFace(glm::vec3 &playerPos, glm::vec3 &eyeSight)
{
    glm::vec3 pointedBlock = findNearestBlock(playerPos, eyeSight);

    if(fequal(pointedBlock[0], -0.1f))
    {
        return pointedBlock;
    }

    glm::vec3 translation = pointedBlock;
    glm::vec3 normalShift = glm::vec3(0.0f, 0.0f, 0.0f);
    float tNear = -1000.0f;

    // Detect x slab
    float t0 = (translation[0] - playerPos[0]) / eyeSight[0];
    float t1 = (translation[0] + 1 - playerPos[0]) / eyeSight[0];


    if(!fequal(t0, t1) && t0 > t1)
    {
        if(!fequal(t1, tNear) && t1 > 0 && t1 > tNear)
        {
            normalShift = glm::vec3(1.0f, 0.0f, 0.0f);
            tNear = t1;
        }
    }
    else
    {
        if(!fequal(t0, tNear) && t0 > 0 && t0 > tNear)
        {
            normalShift = glm::vec3(-1.0f, 0.0f, 0.0f);
            tNear = t0;
        }
    }

    // Detect Y slab
    t0 = (translation[1] - playerPos[1]) / eyeSight[1];
    t1 = (translation[1] + 1 - playerPos[1]) / eyeSight[1];

    if(!fequal(t0, t1) && t0 > t1)
    {
        if(!fequal(t1, tNear) && t1 > 0 && t1 > tNear)
        {
            normalShift = glm::vec3(0.0f, 1.0f, 0.0f);
            tNear = t1;
        }
    }
    else
    {
        if(!fequal(t0, tNear) && t0 > 0 && t0 > tNear)
        {
            normalShift = glm::vec3(0.0f, -1.0f, 0.0f);
            tNear = t0;
        }
    }

    // Detect Z slab
    t0 = (translation[2] - playerPos[2]) / eyeSight[2];
    t1 = (translation[2] + 1 - playerPos[2]) / eyeSight[2];

    if(!fequal(t0, t1) && t0 > t1)
    {
        if(!fequal(t1, tNear) && t1 > 0 && t1 > tNear)
        {
            normalShift = glm::vec3(0.0f, 0.0f, 1.0f);
            tNear = t1;
        }
    }
    else
    {
        if(!fequal(t0, tNear) && t0 > 0 && t0 > tNear)
        {
            normalShift = glm::vec3(0.0f, 0.0f, -1.0f);
            tNear = t0;
        }
    }

    glm::vec3 re = pointedBlock + normalShift;
    //setBlockAt(re.x, re.y, re.z, STONE);

    if(getBlockAt(re.x - offsetX * 16.0f, re.y, re.z - offsetZ * 16.0f) != EMPTY)
    {
        return glm::vec3(-0.1f, -0.1f, -0.1f);
    }

    return re;
}

void Terrain::updateChunkForUpdatedBlock(int x, int y, int z)
{
    glm::ivec2 block = getChunkBlockIndexFromBlockIndex(x, y, z);

    chunks[block[0]]->destroy();
    chunks[block[0]]->create();
    chunks[block[0]]->sendGPUDirectives();

    if(getChunkIndexX(block[0]) != 0 && Chunk::getBlockIndexX(block[1]) == 0)
    {
        chunks[block[0] - 4]->destroy();
        chunks[block[0] - 4]->create();
        chunks[block[0] - 4]->sendGPUDirectives();
    }

    if(getChunkIndexX(block[0]) != 3 && Chunk::getBlockIndexX(block[1]) == 15)
    {
        chunks[block[0] + 4]->destroy();
        chunks[block[0] + 4]->create();
        chunks[block[0] + 4]->sendGPUDirectives();
    }

    if(getChunkIndexZ(block[0]) != 0 && Chunk::getBlockIndexZ(block[1]) == 0)
    {
        chunks[block[0] - 1]->destroy();
        chunks[block[0] - 1]->create();
        chunks[block[0] - 1]->sendGPUDirectives();
    }

    if(getChunkIndexZ(block[0]) != 3 && Chunk::getBlockIndexZ(block[1]) == 15)
    {
        chunks[block[0] + 1]->destroy();
        chunks[block[0] + 1]->create();
        chunks[block[0] + 1]->sendGPUDirectives();
    }
}

void Terrain::playerChange(glm::vec3 pos, BlockType b)
{
    std::tuple<float, float, float> posTuple = std::make_tuple(pos.x, pos.y, pos.z);
    playerOp[posTuple] = b;
}

glm::vec4 clampColor(const glm::vec4 &color)
{
    glm::vec4 clampedColor;
    if (color.x < 0) clampedColor.x = 0;
    else if (color.x > 1) clampedColor.x = 1;
    else clampedColor.x = color.x;

    if (color.y < 0) clampedColor.y = 0;
    else if (color.y > 1) clampedColor.y = 1;
    else clampedColor.y = color.y;

    if (color.z < 0) clampedColor.z = 0;
    else if (color.z > 1) clampedColor.z = 1;
    else clampedColor.z = color.z;

    if (color.w < 0) clampedColor.w = 0;
    else if (color.w > 1) clampedColor.w = 1;
    else clampedColor.w = color.w;

    return clampedColor;
}

void TestCubeSet::create()
{
    std::vector<glm::vec4> vertexBufferData;
    std::vector<GLuint> indexBufferData;


    // interate over every block
    for(int i = 0; i < testCubeTypes.size(); ++i)
    {
        if (testCubeTypes[i] == EMPTY)
        {
            continue;
        }
        if(!isTransparent(testCubeTypes[i]))
        {
            addCubeData(i, vertexBufferData, indexBufferData);
        }
    }
    for(int i = 0; i < testCubeTypes.size(); ++i)
    {
        if (testCubeTypes[i] == EMPTY)
        {
            continue;
        }
        if(isTransparent(testCubeTypes[i]))
        {
            addCubeData(i, vertexBufferData, indexBufferData);
        }
    }

    count = indexBufferData.size();
    if(indexBufferData.empty())
    {
        return;
    }

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec4), vertexBufferData.data(), GL_STATIC_DRAW);
}

void TestCubeSet::addCubeData(int cubeIndex, std::vector<glm::vec4> &vertexBufferData, std::vector<GLuint> &indexBufferData)
{
    glm::vec4 positionOffset(testCubePositions[cubeIndex].x,
                             testCubePositions[cubeIndex].y,
                             testCubePositions[cubeIndex].z, 0.f);

    glm::vec2 uvFront(0, 0), uvBack(0, 0), uvLeft(0, 0), uvRight(0, 0), uvTop(0, 0), uvBottom(0, 0);
    glm::vec4 color;

    float _blinnPower = 0, _isAnimated = 0;

    int blockType = testCubeTypes[cubeIndex];

    if(isAnimated((BlockType)blockType))
    {
        _isAnimated = 1;
    }

    switch(blockType)
    {
    case DIRT:
        _blinnPower = 0;
        color = colorDirt;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(2, 0);
        break;
    case GRASS:
    {
        _blinnPower = 0;
        color = colorGrass;
        glm::vec3 posGrass = testCubePositions[cubeIndex];
        posGrass.y += 1;
        bool hasTop = false;
        for (int i = 0; i < testCubePositions.size(); ++i)
        {
            if (testCubePositions[i] == posGrass)
            {
                hasTop = true;
            }
        }
        if (!hasTop)
        {
            uvFront = uvBack = uvLeft = uvRight = glm::vec2(3, 0);
            uvTop = glm::vec2(8, 2);
            uvBottom = glm::vec2(2, 0);
        }
        else
        {
            uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(2, 0);
        }
        break;
    }
    case STONE:
        _blinnPower = 10;
        color = colorStone;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(1, 0);
        break;
    case LAVA:
        _blinnPower = 30;
        color = colorLava;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(14, 15);
        break;
    case WATER:
        _blinnPower = 50;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(14, 13);
        break;
    case ICE:
        _blinnPower = 30;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(3, 4);
        break;
    case BEDROCK:
        _blinnPower = 10;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(1, 1);
        break;
    case LEAF:
        _blinnPower = 5;
        uvFront = uvBack = uvLeft = uvRight = uvTop = uvBottom = glm::vec2(5, 3);
        break;
    case WOOD:
        _blinnPower = 5;
        uvFront = uvBack = uvLeft = uvRight = glm::vec2(4, 1);
        uvTop = uvBottom = glm::vec2(5, 1);
        break;
    default:
        break;
    }

    // add data for a face to be rendered
    auto addFace = [&vertexBufferData, &indexBufferData, &positionOffset]
            (const glm::vec4 &position_1, const glm::vec4 &position_2, const glm::vec4 &position_3, const glm::vec4 &position_4,
                const glm::vec4 &normal, const glm::vec4 &color, const glm::vec2 &uv, float _blinnPower, float _isAnimated)
    {
        unsigned int startingIndex = vertexBufferData.size() / 4;
        std::array<glm::vec4, 4> additionalInfo;

        // UV info
        additionalInfo[0][0] = uv.x + 1, additionalInfo[0][1] = 16 - uv.y;
        additionalInfo[1][0] = uv.x + 1, additionalInfo[1][1] = 16 - uv.y - 1;
        additionalInfo[2][0] = uv.x, additionalInfo[2][1] = 16 - uv.y - 1;
        additionalInfo[3][0] = uv.x, additionalInfo[3][1] = 16 - uv.y;

        for(int i = 0; i < 4; ++i)
        {
            additionalInfo[i][0] *= 0.0625f;
            additionalInfo[i][1] *= 0.0625f;
        }

        // blinn power
        additionalInfo[0][2] = _blinnPower;
        additionalInfo[1][2] = _blinnPower;
        additionalInfo[2][2] = _blinnPower;
        additionalInfo[3][2] = _blinnPower;

        // is animated
        additionalInfo[0][3] = _isAnimated;
        additionalInfo[1][3] = _isAnimated;
        additionalInfo[2][3] = _isAnimated;
        additionalInfo[3][3] = _isAnimated;

        indexBufferData.insert(indexBufferData.end(),
            { startingIndex, startingIndex + 1, startingIndex + 2,
              startingIndex, startingIndex + 2, startingIndex + 3 });
        vertexBufferData.insert(vertexBufferData.end(),
            { positionOffset + position_1, normal, color, additionalInfo[0],
              positionOffset + position_2, normal, color, additionalInfo[1],
              positionOffset + position_3, normal, color, additionalInfo[2],
              positionOffset + position_4, normal, color, additionalInfo[3] });
    };

    float offset1 = 0.f, offset2 = 0.f, offset3 = 0.f;
    glm::vec4 colorFront = glm::vec4{color.x, color.y, color.z, color.w};
    glm::vec4 colorBack = glm::vec4{color.x + offset1, color.y + offset1, color.z + offset1, color.w};
    glm::vec4 colorLeft = glm::vec4{color.x - offset1, color.y - offset1, color.z - offset1, color.w};
    glm::vec4 colorRight = glm::vec4{color.x + offset2, color.y + offset2, color.z + offset2, color.w};
    glm::vec4 colorTop = glm::vec4{color.x - offset2, color.y - offset2, color.z - offset2, color.w};
    glm::vec4 colorBottom = glm::vec4{color.x + offset3, color.y + offset3, color.z + offset3, color.w};
    colorFront = clampColor(colorFront);
    colorBack = clampColor(colorBack);
    colorLeft = clampColor(colorLeft);
    colorRight = clampColor(colorRight);
    colorTop = clampColor(colorTop);
    colorBottom = clampColor(colorBottom);


    auto addFront = [addFace, colorFront, uvFront, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
                glm::vec4(0.f, 0.f, 1.f, 0.f), colorFront, uvFront, _blinnPower, _isAnimated);
    };

    auto addBack = [addFace, colorBack, uvBack, _blinnPower, _isAnimated] {
        addFace(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(0.f, 0.f, -1.f, 0.f), colorBack, uvBack, _blinnPower, _isAnimated);
    };

    auto addTop = [addFace, colorTop, uvTop, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(0.f, 1.f, 0.f, 0.f), colorTop, uvTop, _blinnPower, _isAnimated);
    };

    auto addBottom = [addFace, colorBottom, uvBottom, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                glm::vec4(0.f, -1.f, 0.f, 0.f), colorBottom, uvBottom, _blinnPower, _isAnimated);
    };

    auto addLeft = [addFace, colorLeft, uvLeft, _blinnPower, _isAnimated] {
        addFace(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
                glm::vec4(-1.f, 0.f, 0.f, 0.f), colorLeft, uvLeft, _blinnPower, _isAnimated);
    };

    auto addRight = [addFace, colorRight, uvRight, _blinnPower, _isAnimated] {
        addFace(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                glm::vec4(1.f, 0.f, 0.f, 0.f), colorRight, uvRight, _blinnPower, _isAnimated);
    };

    std::bitset<6> visibility("111111");

    bool frontEnabled = visibility[0];
    bool backEnabled = visibility[1];
    bool leftEnabled = visibility[2];
    bool rightEnabled = visibility[3];
    bool topEnabled = visibility[4];
    bool bottomEnabled = visibility[5];

    glm::vec3 anticipatedCubePosition = testCubePositions[cubeIndex];
    std::vector<glm::vec3> allPositions = testCubePositions;

    int neighborIndex = -1;
    auto anticipatedPositionInSet = [this, &anticipatedCubePosition, &allPositions, &neighborIndex]() -> bool
    {
        for(int i = 0; i < allPositions.size(); ++i)
        {
            if (allPositions[i] == anticipatedCubePosition)
            {
                neighborIndex = i;
                return true;
            }
        }
//        for(glm::vec3 &pos: testCubePositions)
//        {
//            if (pos == anticipatedCubePosition)
//            {
//                return true;
//            }
//        }
        neighborIndex = -1;
        return false;
    };

    auto shouldAdd = [this](int cubeIndex, int neighborIndex) -> bool
    {
        return false;
        if (neighborIndex == -1)
        {
            return false;
        }
        return !isTransparent(testCubeTypes[cubeIndex]) && isTransparent(testCubeTypes[neighborIndex]);
    };

    anticipatedCubePosition = testCubePositions[cubeIndex];
    if (frontEnabled)
    {
        anticipatedCubePosition.z += 1;
        if (!anticipatedPositionInSet() ) addFront();
    }

    anticipatedCubePosition = testCubePositions[cubeIndex];
    if (backEnabled)
    {
        anticipatedCubePosition.z -= 1;
        if (!anticipatedPositionInSet() ) addBack();
    }

    anticipatedCubePosition = testCubePositions[cubeIndex];
    if (leftEnabled)
    {
        anticipatedCubePosition.x -= 1;
        if (!anticipatedPositionInSet() ) addLeft();
    }

    anticipatedCubePosition = testCubePositions[cubeIndex];
    if (rightEnabled)
    {
        anticipatedCubePosition.x += 1;
        if (!anticipatedPositionInSet() ) addRight();
    }

    anticipatedCubePosition = testCubePositions[cubeIndex];
    if (topEnabled)
    {
        anticipatedCubePosition.y += 1;
        if (!anticipatedPositionInSet()) addTop();
    }

    anticipatedCubePosition = testCubePositions[cubeIndex];
    if (bottomEnabled)
    {
        anticipatedCubePosition.y -= 1;
        if (!anticipatedPositionInSet()) addBottom();
    }
}

void SketchCube::create()
{
    std::vector<glm::vec4> vertexBufferData;
    std::vector<GLuint> indexBufferData;

    std::vector<glm::vec4> points {
        glm::vec4(0, 0, 0, 1),
        glm::vec4(0, 0, 1, 1),
        glm::vec4(1, 0, 1, 1),
        glm::vec4(1, 0, 0, 1),
        glm::vec4(0, 1, 0, 1),
        glm::vec4(0, 1, 1, 1),
        glm::vec4(1, 1, 1, 1),
        glm::vec4(1, 1, 0, 1)
    };

    std::vector<glm::vec4> colors {
        glm::vec4(0, 0, 0, 1),
        glm::vec4(1, 1, 1, 1),
        glm::vec4(1, 0, 0, 1),
        glm::vec4(0, 0, 1, 1),
        glm::vec4(1, 1, 0, 1),
        glm::vec4(0, 1, 0, 1),
        glm::vec4(0.5, 0.5, 0.5, 1),
        glm::vec4(1, 0, 1, 1)
    };

//    glm::vec4 black = glm::vec4(0, 0, 0, 1),
//            white = glm::vec4(1, 1, 1, 1),
//            red = glm::vec4(1, 0, 0, 1),
//            blue = glm::vec4(0, 0, 1, 1),
//            yellow = glm::vec4(1, 1, 0, 1),
//            green = glm::vec4(0, 1, 0, 1),
//            grey = glm::vec4(0.5, 0.5, 0.5, 1),
//            magenta = glm::vec4(1, 0, 1, 1);

    for(int i = 0; i < 8; ++i)
    {
        vertexBufferData.push_back(glm::vec4{position, 0} + points[i]);
        vertexBufferData.push_back(glm::vec4{0, 0, 1, 0});
        vertexBufferData.push_back(colors[i]);
        vertexBufferData.push_back(glm::vec4(0, 0, 0, 0));
    }

    indexBufferData.insert(indexBufferData.end(),
        {0, 1, 1, 2, 2, 3, 3, 0,
         4, 5, 5, 6, 6, 7, 7, 4,
         0, 4, 1, 5, 2, 6, 3, 7});

    count = indexBufferData.size();
    if(indexBufferData.empty())
    {
        return;
    }

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec4), vertexBufferData.data(), GL_STATIC_DRAW);
}

GLenum SketchCube::drawMode()
{
    return GL_LINES;
}

bool isAnimated(BlockType t)
{
    switch (t)
    {
    case LAVA:
    case WATER:
        return true;
    default:
        return false;
    }
}

bool isAnimated(int t)
{
    switch (t)
    {
    case LAVA:
    case WATER:
        return true;
    default:
        return false;
    }
}

bool isTransparent(BlockType t)
{
    switch (t)
    {
    case ICE:
    case LAVA:
    case WATER:
        return true;
    default:
        return false;
    }
}

bool isTransparent(int t)
{
    switch (t)
    {
    case ICE:
    case LAVA:
    case WATER:
        return true;
    default:
        return false;
    }
}
