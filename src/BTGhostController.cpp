/*
 * BTGhostController.cpp
 *
 *  Created on: May 2, 2018
 *      Author: VIDEOJUEGOS UTALCA
 */

#include "BTGhostController.h"
#include "Ghost.h"
#include <iostream>
#include <limits>

Info* Info::info=nullptr;


BTGhostController::BTGhostController(std::shared_ptr<Character> character):Controller(character),root(std::make_shared<Selector>())  {

	//Rama1 Si es comestible, huye
	auto filter1 = std::make_shared<Filter>();
	filter1->addCondition(std::make_shared<Powerpill>());
	filter1->addAction(std::make_shared<Frightened>());
	root->addChild(filter1);

	//Rama2 // Si TimeOut activo, scatter
	auto filter2 = std::make_shared<Filter>();
	filter2->addCondition(std::make_shared<TimeOut>());
	filter2->addAction(std::make_shared<Scatter>());
	root->addChild(filter2);
	//Rama3, siempre perseguir	
	root->addChild(std::make_shared<Chase>());

}

BTGhostController::~BTGhostController() {
	// TODO Auto-generated destructor stub
}

Move BTGhostController::getMove(const GameState& gs){
	Info::getInfo()->in_character=character;
	Info::getInfo()->in_gamestate=&gs;
	root->tick();

	return Info::getInfo()->out_move;
}

TimeOut::TimeOut() : Behavior() {
	lastTime = std::chrono::high_resolution_clock::now();

}

Status TimeOut::update(){
	std::chrono::duration<float> timeStamp = std::chrono::high_resolution_clock::now() - lastTime;
	if( (int)timeStamp.count()%27 < 7){
		return BH_SUCCESS;
	}
		return BH_FAILURE;
}

Status Powerpill::update(){
	auto character = Info::getInfo()->in_character;
	auto ghost = dynamic_cast<Ghost*>(character.get());

	if( ghost!=nullptr && ghost->isEdible()){
		return BH_SUCCESS;
	}
		return BH_FAILURE;
	

}

Status Chase::update(){
	 auto character = Info::getInfo()->in_character;
    auto gs        = Info::getInfo()->in_gamestate;
    auto target    = gs->getMaze().getNodePos(gs->getPacmanPos());
 
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
 
    float minDist = std::numeric_limits<float>::max();
    Move  minMove = PASS;
    for (auto move : moves) {
        if (move == PASS) break;
        float dist = euclid2(target,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if (dist < minDist) { minDist = dist; minMove = move; }
    }
    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}

Frightened::Frightened() : Behavior(), e(rand()), uniform_dist(0,3){

}

Status Frightened::update(){
	//std::cerr << " Frightened \n" ;
	auto character = Info::getInfo()->in_character;
	auto gs = Info::getInfo()->in_gamestate;
	auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	float maxDist = -1.0f;
	Move maxMove = PASS;

	for(auto move : moves){
		if(move == PASS)
		break;
		float dist = euclid2(pacmanPos,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));

		if(dist > maxDist) {
			maxDist = dist;
			maxMove = move;
		}
	}
	
	Info::getInfo()->out_move = maxMove;
	return BH_SUCCESS;
}

Scatter :: Scatter() : Behavior(), target({2,0}) {
	;

}

Status Scatter::update(){
	//std::cerr << " Scatter \n" ;
	auto character = Info::getInfo()->in_character;
	auto gs = Info::getInfo()->in_gamestate;

	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	float minDist = std::numeric_limits<float>::max();
    Move  minMove = PASS;
	for (auto move : moves) {
        if (move == PASS) break;
        float dist = euclid2(target, gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if (dist < minDist) { 
			minDist = dist; 
			minMove = move; 
		}
    }
	Info::getInfo()->out_move = minMove;
	return BH_SUCCESS;

}

//BTPacmanController
BTPacmanController::BTPacmanController(std::shared_ptr<Character> character):Controller(character),root(std::make_shared<Selector>())  {

	//Rama1, Si hay fantasma peligroso cerca huir
	auto filter1 = std::make_shared<Filter>();
	filter1->addCondition(std::make_shared<Powerpill>());
	filter1->addAction(std::make_shared<PacmanFlee>());
    root->addChild(filter1);

	//Rama2, Si hay powerpill cerca, ir por ella
	auto filter2 = std::make_shared<Filter>();
    filter2->addCondition(std::make_shared<PowerPillNearby>());
    filter2->addAction(std::make_shared<GoToPowerPill>());
    root->addChild(filter2);

	//Rama3 default, comer la mas cercana
	root->addChild(std::make_shared<EatPills>());
}

BTPacmanController::~BTPacmanController() {}
 
Move BTPacmanController::getMove(const GameState& gs) {
    Info::getInfo()->in_character = character;
    Info::getInfo()->in_gamestate = &gs;
    root->tick();
    return Info::getInfo()->out_move;
}

// Detecta si hay un fantasma No comestible cerca a menos de 5 casillas, si es comestible, no es peligroso
Status GhostNearby::update() {
    auto gs        = Info::getInfo()->in_gamestate;
    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
    const float DANGER_DIST = 5.0f;
 
    for (int i = 0; i < 4; i++) {
        if (gs->isGhostEdible(i)) continue;
        auto ghostPos = gs->getMaze().getNodePos(gs->getGhostsPos(i));
        if (euclid2(pacmanPos, ghostPos) < DANGER_DIST) {
            return BH_SUCCESS; // 
        }
    }
    return BH_FAILURE;
}

//Detecta si hay una powerpill cerca a menos de 10 casillas, va a buscarla solo si hay fantasmas comestibles cerca
Status PowerPillNearby::update() {
    auto gs         = Info::getInfo()->in_gamestate;
    auto pacmanPos  = gs->getMaze().getNodePos(gs->getPacmanPos());
    auto powerPills = gs->getMaze().getPowerPillPositions();
    const float CLOSE_ENOUGH = 10.0f;
 
    for (auto& pill : powerPills) {
        if (euclid2(pacmanPos, pill) < CLOSE_ENOUGH) {
            return BH_SUCCESS;
        }
    }
    return BH_FAILURE;
}

//Huir del fantasma mas cercano, eligiendo la casilla vecina mas lejos de el
Status PacmanFlee::update() {
    auto gs        = Info::getInfo()->in_gamestate;
    auto character = Info::getInfo()->in_character;
    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
 
    // Encontrar el fantasma peligroso mas cercano
    float   minGhostDist = std::numeric_limits<float>::max();
    std::pair<int,int> closestGhost = {0, 0};
    for (int i = 0; i < 4; i++) {
        if (gs->isGhostEdible(i)) continue;
        auto gPos = gs->getMaze().getNodePos(gs->getGhostsPos(i));
        float d   = euclid2(pacmanPos, gPos);
        if (d < minGhostDist) { minGhostDist = d; closestGhost = gPos; }
    }
 
    // Moverse a la casilla mas lejos de ese fantasma
    std::vector<Move> moves = gs->getMaze().getPossibleMoves(character->getPos());
    float maxDist = -1.0f;
    Move  maxMove = PASS;
    for (auto move : moves) {
        if (move == PASS) break;
        float dist = euclid2(closestGhost,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if (dist > maxDist) { maxDist = dist; maxMove = move; }
    }
    Info::getInfo()->out_move = maxMove;
    return BH_SUCCESS;
}
// Ir hacia la power pill mas cercana
Status GoToPowerPill::update() {
    auto gs         = Info::getInfo()->in_gamestate;
    auto character  = Info::getInfo()->in_character;
    auto pacmanPos  = gs->getMaze().getNodePos(gs->getPacmanPos());
    auto powerPills = gs->getMaze().getPowerPillPositions();
 
    if (powerPills.empty()) return BH_FAILURE;
 
    // Encontrar la power pill mas cercana
    std::pair<int,int> closest = powerPills[0];
    float minD = euclid2(pacmanPos, closest);
    for (auto& pill : powerPills) {
        float d = euclid2(pacmanPos, pill);
        if (d < minD) { minD = d; closest = pill; }
    }
 
    // Moverse hacia ella
    std::vector<Move> moves = gs->getMaze().getPossibleMoves(character->getPos());
    float minDist = std::numeric_limits<float>::max();
    Move  minMove = PASS;
    for (auto move : moves) {
        if (move == PASS) break;
        float dist = euclid2(closest,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if (dist < minDist) { minDist = dist; minMove = move; }
    }
    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}

// ir hacia la pildora normal mas cercana
Status EatPills::update() {
    auto gs        = Info::getInfo()->in_gamestate;
    auto character = Info::getInfo()->in_character;
    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
    auto pills     = gs->getMaze().getPillPositions();
 
    if (pills.empty()) return BH_FAILURE;
 
    // Encontrar la pildora mas cercana
    std::pair<int,int> closest = *pills.begin();
    float minD = euclid2(pacmanPos, closest);
    for (auto& pill : pills) {
        float d = euclid2(pacmanPos, pill);
        if (d < minD) { minD = d; closest = pill; }
    }
 
    // Moverse hacia ella
    std::vector<Move> moves = gs->getMaze().getPossibleMoves(character->getPos());
    float minDist = std::numeric_limits<float>::max();
    Move  minMove = PASS;
    for (auto move : moves) {
        if (move == PASS) break;
        float dist = euclid2(closest,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if (dist < minDist) { minDist = dist; minMove = move; }
    }
    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}