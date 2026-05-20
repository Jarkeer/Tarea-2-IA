#include "BlinkyController.h" 
#include "Ghost.h"
#include <cmath>

BlinkyController::BlinkyController(std::shared_ptr<Character> character):
	Controller(character)
{

}

BlinkyController::~BlinkyController() 
{
		
}

Move
BlinkyController::getMove(const GameState& game)
{

	Ghost* ghost =dynamic_cast<Ghost*>(character.get());
	if(ghost==nullptr)return PASS;	

	// Node Raiz del arbol
	bool isEdible = (ghost != nullptr) && ghost->isEdible();

	std::vector<Move> moves;
	if(character -> getDirection() == PASS)
	{
		moves = game.getMaze().getPossibleMoves(character->getPos());
	}
	else
	{
		moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	// Posicion de Pacman
	auto pacmanPos = game.getMaze().getNodePos(game.getPacmanPos());

	Move bestMove = PASS;

	if(isEdible)
	{
		//Huir llegando a la posicion mas lejana de pacman
		float maxDist = -1.0f;
		for(auto move : moves)
		{
			if(move == PASS) break;
			auto neighborPos = game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), move));
			float dist = sqrt(euclid2(neighborPos, pacmanPos));
			if(dist > maxDist)
			{
				maxDist = dist;
				bestMove = move;
			}
		}
	}else
	{
		//Perseguir a pacman llegando a la posicion mas cercana a pacman
		float minDist = 100000.0f;
		for(auto move : moves)
		{
			if(move == PASS) break;
			auto neighborPos = game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), move));
			float dist = sqrt(euclid2(neighborPos, pacmanPos));
			if(dist < minDist)
			{
				minDist = dist;
				bestMove = move;
			}
		}
	}

	return bestMove;	
}
