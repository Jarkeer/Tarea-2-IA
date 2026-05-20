#include "DTGhostController.h"
#include "Ghost.h"

DTGhostController::DTGhostController(std::shared_ptr<Character> character)
    : Controller(character) {
}
 
DTGhostController::~DTGhostController() {
}

Move DTGhostController::getMove(const GameState& game) 
{
 
    Ghost* ghost = dynamic_cast<Ghost*>(character.get());
    bool isEdible = (ghost != nullptr && ghost->isEdible());
 
    std::vector<Move> moves;
    if (character->getDirection() == PASS)
     {
        moves = game.getMaze().getPossibleMoves(character->getPos());
    } else 
    {
        moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
 
    auto pacmanPos = game.getMaze().getNodePos(game.getPacmanPos());
    Move bestMove = PASS;
 
    if (isEdible) 
    {
        
        float maxDist = -1.0f;
        for (auto move : moves) {
            if (move == PASS) break;
            auto neighborPos = game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), move)
            );
            float dist = euclid2(pacmanPos, neighborPos);
            if (dist > maxDist) 
            { 
                maxDist = dist; 
                bestMove = move; 
            }
        }
    } else {
        // 
        float minDist = 1e9f;
        for (auto move : moves) 
        {
            if (move == PASS) break;
            auto neighborPos = game.getMaze().getNodePos(
                game.getMaze().getNeighbour(character->getPos(), move)
            );
            float dist = euclid2(pacmanPos, neighborPos);
            if (dist < minDist)
             { 
                minDist = dist;
                bestMove = move; 
             }
        }
    }
 
    return bestMove;
}