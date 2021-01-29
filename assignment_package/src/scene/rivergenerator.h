#ifndef RIVERGENERATOR_H
#define RIVERGENERATOR_H
#include <la.h>
#include <stack>
#include <unordered_map>
#include <vector>
#include <memory>

class RiverGenerator;

typedef void (RiverGenerator::*Rule)(void); // Rule function pointers

class Turtle
{
public:
    glm::vec2 pos;  // Current position
    glm::vec2 orient;  // Current orientation
    int recurDepth;  // Current recursive depth
    int riverLength;

public:
    // Constructor
    Turtle();
    Turtle(glm::vec2 _pos, glm::vec2 _orient, int _depth = 0);
};

class RiverGenerator
{
public:
    std::unique_ptr<Turtle> curTurtle;
    std::stack<std::unique_ptr<Turtle>> stateStack;  // State stack
    std::unordered_map<char, std::string> expansionRules; // Expansion rules for axiom
    std::unordered_map<char, Rule> drawRules; // Draw rules for axiom
    glm::vec4 generationBound; // River generation bound
    std::vector<glm::vec2> riverVertices;  // River vertices
    std::unordered_map<long long int, int> waterCubes;
    double turningEpsilon;
    int riverWidth;

public:
    // Constructor
    RiverGenerator();
    RiverGenerator(glm::vec2 startPoint, glm::vec2 startOrient);

    // Generate a river by L-system
    void generate(std::string genString);
    std::string expansion(std::string genString, int depth);

    // River generate rule functions
    void fDraw();       // Axiom F: Move forward;
    void plusDraw();    // Axiom +: Rotate 30 degrees
    void minusDraw();   // Axiom -: Rotate -30 degrees
    void saveDraw();    // Axiom [: Save current state
    void reStoreDraw(); // Axiom ]: Restore saved state;
    void lDraw();       // Axiom L: Turn left
    void rDraw();       // Axiom R: Turn Right

    void addDeltaRiverExpansionRules(); // River delta expansionRules setting
    void addLinearRiverExpansionRules(); // River linear expansionRules setting
    void addRiverGenerationRules();  // River generationRules setting
};

#endif // RIVERGENERATOR_H
