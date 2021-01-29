#ifndef BUILDINGGENERATOR_H
#define BUILDINGGENERATOR_H
#include <la.h>
#include <stack>
#include <unordered_map>
#include <vector>
#include <memory>

class BuildingGenerator;

typedef void (BuildingGenerator::*BuildRule)(void); // Rule function pointers

class BuildingTurtle
{
public:
    glm::vec2 pos;  // Current position
    glm::vec2 orient;  // Current orientation
    int recurDepth;  // Current recursive depth
    int buildingHeight;
    int height;

public:
    // Constructor
    BuildingTurtle();
    BuildingTurtle(glm::vec2 _pos, glm::vec2 _orient, int _depth = 0, int _height = 0);

};

class BuildingGenerator
{
public:
    std::unique_ptr<BuildingTurtle> curTurtle;
    std::stack<std::unique_ptr<BuildingTurtle>> stateStack;  // State stack
    std::unordered_map<char, std::string> expansionRules; // Expansion rules for axiom
    std::unordered_map<char, BuildRule> drawRules; // Draw rules for axiom
    glm::vec4 generationBound; // River generation bound
    std::unordered_map<long long int, int> buildingBlock;
    double turningEpsilon;
    //int riverWidth;

public:
    // Constructor
    BuildingGenerator();
    BuildingGenerator(glm::vec2 startPoint, glm::vec2 startOrient);

    // Generate a river by L-system
    void generate(std::string genString);
    std::string expansion(std::string genString, int depth);

    // River generate rule functions
    void mDraw();       // Axiom M: Main room
    void lDraw();       // Axiom L: Corridor Turn Left
    void rDraw();       // Axiom R: Corridor Turn Right
    void fDraw();       // Axiom F: Corridor Forward;
    void bDraw();       // Axiom B" Corridor Back;
    void uDraw();       // Axiom U: UpStairs
    void saveDraw();    // Axiom [
    void restoreDraw(); // Axion ]

    void addBuildingExpansionRules(); // River delta expansionRules setting
    void addBuildingGenerationRules();  // River generationRules setting
};

#endif // BULDINGGENERATOR_H
