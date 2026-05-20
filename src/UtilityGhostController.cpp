#include "UtilityGhostController.h"
#include "Ghost.h"
#include <limits>
 
UtilityGhostController::UtilityGhostController(std::shared_ptr<Character> character)
    : Controller(character) {}
 
UtilityGhostController::~UtilityGhostController() {}
 
float UtilityGhostController::getDistanceToPacman(const GameState& game) const {
    auto myPos  = game.getMaze().getNodePos(character->getPos());
    auto pacPos = game.getMaze().getNodePos(game.getPacmanPos());
    return std::sqrt((float)euclid2(myPos, pacPos));
}
 
Move UtilityGhostController::getClosestMove(const GameState& game,std::pair<int,int> target) const {
    std::vector<Move> moves;
    if (character->getDirection() == PASS)
        moves = game.getMaze().getPossibleMoves(character->getPos());
    else
        moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
 
    float minD = std::numeric_limits<float>::max();
    Move  best = PASS;
    for (auto m : moves) {
        if (m == PASS) break;
        int nb = game.getMaze().getNeighbour(character->getPos(), m);
        float d = (float)euclid2(game.getMaze().getNodePos(nb), target);
        if (d < minD) { minD = d; best = m; }
    }
    return best;
}
 
Move UtilityGhostController::getFarthestMove(const GameState& game,std::pair<int,int> target) const {
    std::vector<Move> moves;
    if (character->getDirection() == PASS)
        moves = game.getMaze().getPossibleMoves(character->getPos());
    else
        moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
 
    float maxD = -1.0f;
    Move  best = PASS;
    for (auto m : moves) {
        if (m == PASS) break;
        int nb = game.getMaze().getNeighbour(character->getPos(), m);
        float d = (float)euclid2(game.getMaze().getNodePos(nb), target);
        if (d > maxD) { maxD = d; best = m; }
    }
    return best;
}
 
 // Fantasma cobarde
Move UtilityGhostController::getMove(const GameState& game) {
 
    Ghost* ghost = dynamic_cast<Ghost*>(character.get());
    auto   pacPos = game.getMaze().getNodePos(game.getPacmanPos());
    float  d = getDistanceToPacman(game);
 
    // Si es comestible, huir siempre
    if (ghost != nullptr && ghost->isEdible()) {
        return getFarthestMove(game, pacPos);
    }
 
    // Curva de perseguir: lineal creciente con la distancia  a más distancia, más utilidad de perseguir
    float uChase = d / 30.0f;
 
    // Curva de huir: lineal decreciente con la distancia a menos distancia, más utilidad de huir 
    
    float uFlee  = 1.0f - d / 30.0f;
 
    // Clamp entre 0 y 1 por si la distancia supera 30
    if (uChase > 1.0f) uChase = 1.0f;
    if (uFlee  < 0.0f) uFlee  = 0.0f;
 
    if (uFlee > uChase)
        return getFarthestMove(game, pacPos);
    else
        return getClosestMove (game, pacPos);
}
