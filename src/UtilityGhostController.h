#ifndef UTILITYGHOSTCONTROLLER_H_
#define UTILITYGHOSTCONTROLLER_H_
 
#include "Controller.h"
#include <cmath>

class UtilityGhostController : public Controller {
 
    float getDistanceToPacman(const GameState& game) const;
    Move  getClosestMove (const GameState& game, std::pair<int,int> target) const;
    Move  getFarthestMove(const GameState& game, std::pair<int,int> target) const;
 
public:
    UtilityGhostController(std::shared_ptr<Character> character);
    virtual ~UtilityGhostController();
    virtual Move getMove(const GameState& game) override;
};



#endif