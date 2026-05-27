#include "UtilityPacmanController.h"
#include <limits>
#include <iostream>
 
UtilityPacmanController::UtilityPacmanController(std::shared_ptr<Character> character)
    : Controller(character) {}
 
UtilityPacmanController::~UtilityPacmanController() {}

// Distancia euclidiana entre el pacman y un objetivo
float UtilityPacmanController::distTo(const GameState& game,std::pair<int,int> target) const {

    auto myPos = game.getMaze().getNodePos(character->getPos());
    return std::sqrt((float)euclid2(myPos, target));
}

// Devulve el movimiento que acerca mas a un objetivo
Move UtilityPacmanController::getClosestMove(const GameState& game,std::pair<int,int> target) const {
    float minD = std::numeric_limits<float>::max();
    Move  best = character->getDirection();
    for (auto m : game.getMaze().getPossibleMoves(character->getPos()))
     {
        int  nb   = game.getMaze().getNeighbour(character->getPos(), m);
        if  (nb < 0) continue;

        float d   = (float)euclid2(game.getMaze().getNodePos(nb), target);
        if  (d < minD) { minD = d; best = m; }
    }
    return best;
}

// Devulve el movimiento que aleja mas de un objetivo
Move UtilityPacmanController::getFarthestMove(const GameState& game,std::pair<int,int> target) const {
    float maxD = -1.0f;
    Move  best = character->getDirection();
    for (auto m : game.getMaze().getPossibleMoves(character->getPos())) 
    {
        int  nb   = game.getMaze().getNeighbour(character->getPos(), m);
        if  (nb < 0) continue;

        float d   = (float)euclid2(game.getMaze().getNodePos(nb), target);
        if  (d > maxD) { maxD = d; best = m; }
    }
    return best;
}

// Curvas de Utilidad 
float UtilityPacmanController::utilityEscape(const GameState& game, int g) const 
{
    if (game.isGhostEdible(g)) return 0.0f; 
    float d = distTo(game, game.getMaze().getNodePos(game.getGhostsPos(g)));
    return 1.0f / (1.0f + std::exp(0.4f * (d - 8.0f)));
}

// Curva para ir a cazar fantasmas comestibles, solo se activa si el fantasma es comestible ira bajando suavemente la distancia y si esta muy lejos no vale la pena perseguirlo
float UtilityPacmanController::utilityHunt(const GameState& game, int g) const 
{
    if (!game.isGhostEdible(g)) return 0.0f;
    float d = distTo(game, game.getMaze().getNodePos(game.getGhostsPos(g)));
    float u = 1.0f - (d / 30.0f) * (d / 30.0f);
    return std::max(0.0f, u);
}
 
// Curva para ir a comer los power pills, 
float UtilityPacmanController::utilitySeekPower(const GameState& game) const 
{
    auto pills = game.getMaze().getPowerPillPositions();
    if (pills.empty()) return 0.0f;

    float danger = 0.0f;
    for (int i = 0; i < 4; i++) {
        danger = std::max(danger, utilityEscape(game, i));
    }
    if (danger < 0.2f) return 0.0f;

    //PowerPill mas cercana
    auto myPos  = game.getMaze().getNodePos(character->getPos());
    float minD  = std::numeric_limits<float>::max();
    for (auto& p : pills)
     {
        float d = std::sqrt((float)euclid2(myPos, p));
        if (d < minD) minD = d;
    }
    if (minD > 25.0f) return 0.0f;

    //Lineal : mas cerca mas utilidad
    float u = danger * (1.0f - minD / 25.0f);
    return std::max(0.0f, u);
}

// curva constante baja para comer las pastillas 

float UtilityPacmanController::utilityEatPills(const GameState& game) const 
{
    auto pills = game.getMaze().getPillPositions();
    if (pills.empty()) return 0.0f;
    float ratio = 1.0f - std::min(1.0f, (float)pills.size() / 200.0f);
    return 0.15f + 0.1f * ratio;
}



// Decisiones principales y finales

Move UtilityPacmanController::getMove(const GameState& game) {
 
    float bestUtil = -1.0f;
    Move  bestMove = character->getDirection();

    // Evaluar es escape de cada fanstama
    for (int i = 0; i < 4; i++) {
        float u = utilityEscape(game, i);
        if (u > bestUtil) {
            bestUtil = u;
            bestMove = getFarthestMove(game,
                game.getMaze().getNodePos(game.getGhostsPos(i)));
        }
    }
    // Evañuar hunt para cada fantasma
    for (int i = 0; i < 4; i++) {
        float u = utilityHunt(game, i);
        if (u > bestUtil) {
            bestUtil = u;
            bestMove = getClosestMove(game,
                game.getMaze().getNodePos(game.getGhostsPos(i)));
        }
    }
        // Evaluar Seekpower
    {
        float u = utilitySeekPower(game);
        if (u > bestUtil) {
            bestUtil = u;
            auto pills  = game.getMaze().getPowerPillPositions();
            auto myPos  = game.getMaze().getNodePos(character->getPos());
            auto closest = pills[0];
            float minD  = std::sqrt((float)euclid2(myPos, pills[0]));
            for (auto& p : pills) {
                float d = std::sqrt((float)euclid2(myPos, p));
                if (d < minD) { minD = d; closest = p; }
            }
            bestMove = getClosestMove(game, closest);
        }
    }
    // Evaluar Comer pastillas

    {
        float u = utilityEatPills(game);
        if (u > bestUtil) {
            bestUtil = u;
            auto pills  = game.getMaze().getPillPositions();
            if (!pills.empty()) {
                auto myPos  = game.getMaze().getNodePos(character->getPos());
                auto closest = pills[0];
                float minD  = std::sqrt((float)euclid2(myPos, pills[0]));
                for (auto& p : pills) {
                    float d = std::sqrt((float)euclid2(myPos, p));
                    if (d < minD) { minD = d; closest = p; }
                }
                bestMove = getClosestMove(game, closest);
            }
        }
    }
 
    return bestMove;
}

