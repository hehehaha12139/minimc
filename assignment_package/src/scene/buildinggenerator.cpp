#include "buildinggenerator.h"
#include <time.h>
#include "utils.h"

BuildingTurtle::BuildingTurtle():recurDepth(0) {}

BuildingTurtle::BuildingTurtle(glm::vec2 _pos, glm::vec2 _orient, int _depth, int _height):
    pos(_pos), orient(_orient), recurDepth(_depth), height(_height){}

BuildingGenerator::BuildingGenerator() {}

BuildingGenerator::BuildingGenerator(glm::vec2 startPoint, glm::vec2 startOrient)
{
    curTurtle = std::make_unique<BuildingTurtle>(startPoint, startOrient);
    addBuildingExpansionRules();
    addBuildingGenerationRules();
}

void BuildingGenerator::generate(std::string genString)
{
    // Traverse each character
    for(int i = 0; i < genString.size(); i++)
    {
        char curChar = genString[i];
        if(drawRules.find(curChar) != drawRules.end())
        {
            (this->*drawRules[curChar])();
        }
    }
}

std::string BuildingGenerator::expansion(std::string genString, int depth)
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


void BuildingGenerator::mDraw()
{
    curTurtle->height = 0;
    glm::vec2 nextPoint = curTurtle->pos;
    for(int i = -10; i <= 10; i++)
    {
        for(int j = -10; j <= 10; j++)
        {
            long long int mainRoomBlockIndex = ((long long int)(nextPoint.x + i) << 32) + (long long int)(nextPoint.y + j);
            if(abs(i) <= 3 && abs(j) <= 3)
            {
                buildingBlock[mainRoomBlockIndex] = curTurtle->height;
            }
            else
            {
                buildingBlock[mainRoomBlockIndex] = -5;
            }
        }
    }
}

void BuildingGenerator::uDraw()
{
    curTurtle->height += 1;
    glm::vec2 nextPoint = curTurtle->pos;
    for(int i = -10; i  <= 10; i++)
    {
        for(int j = -10; j <= -10; j++)
        {
            long long int mainRoomBlockIndex = ((long long int)(nextPoint.x + i) << 32) + (long long int)(nextPoint.y + j);
            if(abs(i) <= 3 && abs(j) <= 3)
            {
                buildingBlock[mainRoomBlockIndex] = curTurtle->height;
            }
            else
            {
                buildingBlock[mainRoomBlockIndex] = -5;
            }
        }
    }

}

void BuildingGenerator::saveDraw()
{
    stateStack.push(std::move(curTurtle));
    curTurtle = std::make_unique<BuildingTurtle>(stateStack.top()->pos,
                                         stateStack.top()->orient,
                                         stateStack.top()->recurDepth + 1);
}

void BuildingGenerator::restoreDraw()
{
    curTurtle = std::move(stateStack.top());
    stateStack.pop();
}

void BuildingGenerator::lDraw()
{
    srand(time(NULL));
    int length = (rand() % (10 - 5 + 1)) + 5;
    curTurtle->pos = curTurtle->pos + glm::vec2(-4, 0);
    for(int i = 0; i < length; i++)
    {
        for(int i = -8; i <= 8; i++)
        {
            glm::vec2 nextPoint = curTurtle->pos;
            long long int corridorBlockIndex = ((long long int)(nextPoint.x) << 32) + (long long int)(nextPoint.y + i);
            if(abs(i) <= 1)
            {
                buildingBlock[corridorBlockIndex] = -1;
            }
            else if(abs(i) <= 2)
            {
                buildingBlock[corridorBlockIndex] = -2;
            }
            else
            {
                buildingBlock[corridorBlockIndex] = -5;
            }
        }

        curTurtle->pos = curTurtle->pos + glm::vec2(-1, 0);
    }
    curTurtle->pos = curTurtle->pos + glm::vec2(-4, 0);
}

void BuildingGenerator::rDraw()
{
    srand(time(NULL));
    int length = (rand() % (10 - 5 + 1)) + 5;
    curTurtle->pos = curTurtle->pos + glm::vec2(4, 0);
    for(int i = 0; i < length; i++)
    {
        for(int i = -8; i <= 8; i++)
        {
            glm::vec2 nextPoint = curTurtle->pos;
            long long int corridorBlockIndex = ((long long int)(nextPoint.x) << 32) + (long long int)(nextPoint.y + i);
            if(abs(i) <= 1)
            {
                buildingBlock[corridorBlockIndex] = -1;
            }
            else if(abs(i) <= 2)
            {
                buildingBlock[corridorBlockIndex] = -2;
            }
            else
            {
                buildingBlock[corridorBlockIndex] = -5;
            }
        }
        curTurtle->pos = curTurtle->pos + glm::vec2(1, 0);
    }
    curTurtle->pos = curTurtle->pos + glm::vec2(4, 0);
}

void BuildingGenerator::fDraw()
{
    srand(time(NULL));
    int length = (rand() % (10 - 5 + 1)) + 5;
    curTurtle->pos = curTurtle->pos + glm::vec2(0, 4);
    for(int i = 0; i < length; i++)
    {
        for(int i = -8; i <= 8; i++)
        {
            glm::vec2 nextPoint = curTurtle->pos;
            long long int corridorBlockIndex = ((long long int)(nextPoint.x) << 32 + i) + (long long int)(nextPoint.y);
            if(abs(i) <= 1)
            {
                buildingBlock[corridorBlockIndex] = -1;
            }
            else if(abs(i) <= 2)
            {
                buildingBlock[corridorBlockIndex] = -2;
            }
            else
            {
                buildingBlock[corridorBlockIndex] = -5;
            }
        }
        curTurtle->pos = curTurtle->pos + glm::vec2(0, 1);
    }
    curTurtle->pos = curTurtle->pos + glm::vec2(0, 4);
}

void BuildingGenerator::bDraw()
{
    srand(time(NULL));
    int length = (rand() % (10 - 5 + 1)) + 5;
    curTurtle->pos = curTurtle->pos + glm::vec2(0, -4);
    for(int i = 0; i < length; i++)
    {
        for(int i = -8; i <= 8; i++)
        {
            glm::vec2 nextPoint = curTurtle->pos;
            long long int corridorBlockIndex = ((long long int)(nextPoint.x + i) << 32) + (long long int)(nextPoint.y);
            if(abs(i) <= 1)
            {
                buildingBlock[corridorBlockIndex] = -1;
            }
            else if(abs(i) <= 2)
            {
                buildingBlock[corridorBlockIndex] = -2;
            }
            else
            {
                buildingBlock[corridorBlockIndex] = -5;
            }
        }
        curTurtle->pos = curTurtle->pos + glm::vec2(0, -1);
    }
    curTurtle->pos = curTurtle->pos + glm::vec2(0, -4);
}


void BuildingGenerator::addBuildingExpansionRules()
{
    expansionRules['M'] = "M[U][L][R][B]";
    expansionRules['L'] = "LM";
    expansionRules['R'] = "RM";
    expansionRules['F'] = "FM";
    expansionRules['B'] = "BM";
}

void BuildingGenerator::addBuildingGenerationRules()
{
    drawRules['M'] = &mDraw;
    drawRules['L'] = &lDraw;
    drawRules['R'] = &rDraw;
    drawRules['F'] = &fDraw;
    drawRules['B'] = &bDraw;
    drawRules['U'] = &uDraw;
    drawRules['['] = &saveDraw;
    drawRules[']'] = &restoreDraw;
}


