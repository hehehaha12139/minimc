#include "rivergenerator.h"
#include <time.h>
#include "utils.h"

Turtle::Turtle():recurDepth(0) {}

Turtle::Turtle(glm::vec2 _pos, glm::vec2 _orient, int _depth):
    pos(_pos), orient(_orient), recurDepth(_depth){}

RiverGenerator::RiverGenerator() {}

RiverGenerator::RiverGenerator(glm::vec2 startPoint, glm::vec2 startOrient):
    turningEpsilon(1.0), riverWidth(4)
{
    curTurtle = std::make_unique<Turtle>(startPoint, startOrient);
    addLinearRiverExpansionRules();
    addRiverGenerationRules();
}

void RiverGenerator::generate(std::string genString)
{
    // Traverse each character
    waterCubes.clear();
    for(int i = 0; i < genString.size(); i++)
    {
        char curChar = genString[i];
        if(drawRules.find(curChar) != drawRules.end())
        {
            (this->*drawRules[curChar])();
        }
    }
}

std::string RiverGenerator::expansion(std::string genString, int depth)
{
    std::string returnStr = genString;
    for(int j = 0; j < depth; j++)
    {
        std::string resultStr = "";
        for(int i = 0; i < returnStr.size(); i++)
        {
            if(expansionRules.find(returnStr[i]) != expansionRules.end())
            {
                resultStr += expansionRules[returnStr[i]];
            }
            else
            {
                resultStr += returnStr[i];
            }
        }
        returnStr = resultStr;
    }
    return returnStr;
}

void RiverGenerator::fDraw()
{
    srand(time(NULL));
    int forwardStep = (rand() % (15 - 5 + 1)) + 5;
    glm::vec2 curPos = curTurtle->pos;
    glm::vec2 curOrient = curTurtle->orient;
    glm::vec2 lastPoint = curPos;
    glm::vec2 newPoint;
    glm::mat2 radiusRot = glm::mat2(
                            glm::vec2(0.0f, -1.0f),
                            glm::vec2(1.0f, 0.0f)
                          );
    for(int i = 0, count = 0; i < forwardStep; i++, count++)
    {
        glm::vec2 radius = radiusRot * curOrient;
        newPoint = curPos + float(count) * curOrient;
        newPoint = glm::vec2(floor(newPoint.x), floor(newPoint.y));
        if(fequal(newPoint.x, lastPoint.x) && fequal(newPoint.y, lastPoint.y))
        {
            i--;
            continue;
        }
        lastPoint = newPoint;
        for(int j = -(riverWidth + 12); j <= riverWidth + 12; j++)
        {
            std::vector<long long int> waterCubeIndices;
            long long int waterCubeIndex;
            if(!fequal(fabs(curOrient.x), fabs(curOrient.y), turningEpsilon) && fabs(curOrient.x) > fabs(curOrient.y))
            {
                waterCubeIndex = ((long long int)(newPoint.x) << 32) + (long long int)(newPoint.y + j);
                waterCubeIndices.push_back(waterCubeIndex);
            }
            else if (!fequal(fabs(curOrient.x), fabs(curOrient.y), turningEpsilon) && fabs(curOrient.y) > fabs(curOrient.x))
            {
                waterCubeIndex = ((long long int)(newPoint.x + j) << 32) + (long long int)newPoint.y;
                waterCubeIndices.push_back(waterCubeIndex);
            }
            else
            {
                waterCubeIndex = ((long long int)(newPoint.x + j) << 32) + (long long int)(newPoint.y + j);
                waterCubeIndices.push_back(waterCubeIndex);
                waterCubeIndices.push_back(((long long int)(newPoint.x) << 32) + (long long int)(newPoint.y + j));
                waterCubeIndices.push_back(((long long int)(newPoint.x + j) << 32) + (long long int)(newPoint.y));
            }

            for(int z = 0; z < waterCubeIndices.size(); z++)
            {
                waterCubeIndex = waterCubeIndices[z];
                if(waterCubes.find(waterCubeIndex) != waterCubes.end())
                {

                    if(waterCubes[waterCubeIndex] < riverWidth - abs(j))
                    {
                        waterCubes[waterCubeIndex] = riverWidth - abs(j);
                    }


                }
                else
                {
                    waterCubes[waterCubeIndex] = riverWidth - abs(j);
                }
            }

        }
    }
    curTurtle->pos = newPoint;
}

void RiverGenerator::plusDraw()
{
    srand(time(NULL));
    float orientAngle = (rand() % (50 - 20 + 1)) + 20;
    glm::mat2 orientChange = glm::mat2(
                    glm::vec2(cos(glm::radians(orientAngle)), -sin(glm::radians(orientAngle))),
                    glm::vec2(sin(glm::radians(orientAngle)), cos(glm::radians(orientAngle)))
                );
    curTurtle->orient = orientChange * curTurtle->orient;
}

void RiverGenerator::minusDraw()
{
    srand(time(NULL));
    float orientAngle = -((rand() % (50 - 20 + 1)) + 20);
    glm::mat2 orientChange = glm::mat2(
                    glm::vec2(cos(glm::radians(orientAngle)), -sin(glm::radians(orientAngle))),
                    glm::vec2(sin(glm::radians(orientAngle)), cos(glm::radians(orientAngle)))
                );
    curTurtle->orient = orientChange * curTurtle->orient;
}

void RiverGenerator::saveDraw()
{
    stateStack.push(std::move(curTurtle));
    curTurtle = std::make_unique<Turtle>(stateStack.top()->pos,
                                         stateStack.top()->orient,
                                         stateStack.top()->recurDepth + 1);
}

void RiverGenerator::reStoreDraw()
{
    curTurtle = std::move(stateStack.top());
    stateStack.pop();
}

void RiverGenerator::lDraw()
{
    srand(time(NULL));
    int radius = (rand() % (30 - 10 + 1)) + 10;
    int angle = (rand() % (90 - 30 + 1)) + 30;
    glm::mat2 radiusRot = glm::mat2(
                            glm::vec2(0.0f, -1.0f),
                            glm::vec2(1.0f, 0.0f)
                          );
    glm::vec2 turnCentralVec = radiusRot * curTurtle->orient;
    glm::vec2 turnCentral = curTurtle->pos + (float)radius * turnCentralVec;
    turnCentral = glm::vec2(floor(turnCentral.x), floor(turnCentral.y));
    glm::vec2 lastPoint = curTurtle->pos;
    glm::vec2 lastExactPoint = curTurtle->pos;
    for(int i = 0; i < radius * 5; i++)
    {
        float nextAngle = (float)angle / ((float)radius * 5);
        glm::mat2 nextRot = glm::mat2(
                                glm::vec2(cos(glm::radians(nextAngle)), -sin(glm::radians(nextAngle))),
                                glm::vec2(sin(glm::radians(nextAngle)), cos(glm::radians(nextAngle)))
                             );
        glm::vec2 curRadius = lastExactPoint - turnCentral;
        glm::vec2 nextRadius = nextRot * curRadius;
        lastExactPoint = turnCentral + nextRadius;
        glm::vec2 nextPoint = glm::vec2(floor(lastExactPoint.x), floor(lastExactPoint.y));
        if(fequal(nextPoint.x, lastPoint.x) && fequal(nextPoint.y, lastPoint.y))
        {
            continue;
        }
        else
        {
            for(int j = -(riverWidth + 12); j <= riverWidth + 12; j++)
            {
                std::vector<long long int> waterCubeIndices;
                glm::vec2 flowVertical = glm::normalize(nextRadius);
                long long int waterCubeIndex;
                if(!fequal(fabs(flowVertical.x), fabs(flowVertical.y), turningEpsilon) && fabs(flowVertical.y) > fabs(flowVertical.x))
                {
                    waterCubeIndex = ((long long int)(nextPoint.x) << 32) + (long long int)(nextPoint.y + j);
                    waterCubeIndices.push_back(waterCubeIndex);
                }
                else if(!fequal(fabs(flowVertical.x), fabs(flowVertical.y), turningEpsilon) && fabs(flowVertical.x) > fabs(flowVertical.y))
                {
                    waterCubeIndex = ((long long int)(nextPoint.x + j) << 32) + (long long int)nextPoint.y;
                    waterCubeIndices.push_back(waterCubeIndex);
                }
                else
                {
                    waterCubeIndex = ((long long int)(nextPoint.x + j) << 32) + (long long int)(nextPoint.y + j);
                    waterCubeIndices.push_back(waterCubeIndex);
                    waterCubeIndices.push_back(((long long int)(nextPoint.x) << 32) + (long long int)(nextPoint.y + j));
                    waterCubeIndices.push_back(((long long int)(nextPoint.x + j) << 32) + (long long int)(nextPoint.y));
                }

                for(int z = 0; z < waterCubeIndices.size(); z++)
                {
                    waterCubeIndex = waterCubeIndices[z];
                    if(waterCubes.find(waterCubeIndex) != waterCubes.end())
                    {

                        if(waterCubes[waterCubeIndex] < riverWidth - abs(j))
                        {
                            waterCubes[waterCubeIndex] = riverWidth - abs(j);
                        }
                    }
                    else
                    {
                       waterCubes[waterCubeIndex] = riverWidth - abs(j);
                    }
                }
            }
            lastPoint = nextPoint;
        }
        curTurtle->pos = lastPoint;

    }

}

void RiverGenerator::rDraw()
{
    srand(time(NULL));
    int radius = (rand() % (30 - 10 + 1)) + 10;
    int angle = (rand() % (90 - 30 + 1)) + 30;
    glm::mat2 radiusRot = glm::mat2(
                            glm::vec2(0.0f, 1.0f),
                            glm::vec2(-1.0f, 0.0f)
                          );
    glm::vec2 turnCentralVec = radiusRot * curTurtle->orient;
    glm::vec2 turnCentral = curTurtle->pos + (float)radius * turnCentralVec;
    turnCentral = glm::vec2(floor(turnCentral.x), floor(turnCentral.y));
    glm::vec2 lastPoint = curTurtle->pos;
    glm::vec2 lastExactPoint = curTurtle->pos;
    for(int i = 0; i < radius * 5; i++)
    {
        float nextAngle = -(float)angle / ((float)radius * 5);
        glm::mat2 nextRot = glm::mat2(
                                glm::vec2(cos(glm::radians(nextAngle)), -sin(glm::radians(nextAngle))),
                                glm::vec2(sin(glm::radians(nextAngle)), cos(glm::radians(nextAngle)))
                             );
        glm::vec2 curRadius = lastExactPoint - turnCentral;
        glm::vec2 nextRadius = nextRot * curRadius;
        lastExactPoint = turnCentral + nextRadius;
        glm::vec2 nextPoint = glm::vec2(floor(lastExactPoint.x), floor(lastExactPoint.y));
        if(fequal(nextPoint.x, lastPoint.x) && fequal(nextPoint.y, lastPoint.y))
        {
            continue;
        }
        else
        {

            for(int j = -(riverWidth + 12); j <= riverWidth + 12; j++)
            {
                std::vector<long long int> waterCubeIndices;
                glm::vec2 flowVertical = glm::normalize(nextRadius);
                long long int waterCubeIndex;
                if(!fequal(fabs(flowVertical.x), fabs(flowVertical.y), turningEpsilon) && fabs(flowVertical.y) > fabs(flowVertical.x))
                {
                    waterCubeIndex = ((long long int)(nextPoint.x) << 32) + (long long int)(nextPoint.y + j);
                    waterCubeIndices.push_back(waterCubeIndex);
                }
                else if(!fequal(fabs(flowVertical.x), fabs(flowVertical.y), turningEpsilon) && fabs(flowVertical.x) > fabs(flowVertical.y))
                {
                    waterCubeIndex = ((long long int)(nextPoint.x + j) << 32) + (long long int)nextPoint.y;
                    waterCubeIndices.push_back(waterCubeIndex);
                }
                else
                {
                    waterCubeIndex = ((long long int)(nextPoint.x + j) << 32) + (long long int)(nextPoint.y + j);
                    waterCubeIndices.push_back(waterCubeIndex);
                    waterCubeIndices.push_back(((long long int)(nextPoint.x) << 32) + (long long int)(nextPoint.y + j));
                    waterCubeIndices.push_back(((long long int)(nextPoint.x + j) << 32) + (long long int)(nextPoint.y));
                }

                for(int z = 0; z < waterCubeIndices.size(); z++)
                {
                    waterCubeIndex = waterCubeIndices[z];
                    if(waterCubes.find(waterCubeIndex) != waterCubes.end())
                    {
                        if(waterCubes[waterCubeIndex] < riverWidth - abs(j))
                        {
                            waterCubes[waterCubeIndex] = riverWidth - abs(j);
                        }
                    }
                    else
                    {
                        waterCubes[waterCubeIndex] = riverWidth - abs(j);
                    }
                }
            }
            lastPoint = nextPoint;
        }
        curTurtle->pos = lastPoint;
    }
}


void RiverGenerator::addDeltaRiverExpansionRules()
{
    expansionRules['F'] = "F";
    expansionRules['L'] = "L[FL][FR]";
    expansionRules['R'] = "R[FL][FR]";
}

void RiverGenerator::addRiverGenerationRules()
{
    drawRules['F'] = &fDraw;
    drawRules['+'] = &plusDraw;
    drawRules['-'] = &minusDraw;
    drawRules['['] = &saveDraw;
    drawRules[']'] = &reStoreDraw;
    drawRules['L'] = &lDraw;
    drawRules['R'] = &rDraw;
}

void RiverGenerator::addLinearRiverExpansionRules()
{
    expansionRules['F'] = "FLR";
    expansionRules['L'] = "LRF";
    expansionRules['R'] = "RFL";
}
