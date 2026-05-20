#ifndef UTILITYPACMANCONTROLLER_H_
#define UTILITYPACMANCONTROLLER_H_
 
#include "Controller.h"
#include <cmath>

class UtilityPacmanController : public Controller {
 
    Move getClosestMove(const GameState& game, std::pair<int,int> target) const;
    Move getFarthestMove(const GameState& game, std::pair<int,int> target) const;
    float distTo(const GameState& game, std::pair<int,int> target) const;
 
    float utilityEscape(const GameState& game, int ghostIdx) const;
    float utilityHunt(const GameState& game, int ghostIdx) const;
    float utilitySeekPower(const GameState& game) const;
    float utilityEatPills(const GameState& game) const;
 
public:
    UtilityPacmanController(std::shared_ptr<Character> character);
    virtual ~UtilityPacmanController();
    virtual Move getMove(const GameState& game) override;
};
 
#endif