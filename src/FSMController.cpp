    /*
    * FSMController.cpp
    *
    *  Created on: Apr 23, 2018
    *      Author: nbarriga
    */

#include "FSMController.h"
#include "Ghost.h"
#include <iostream>

FSMController::FSMController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<BlinkyStateMachine>(character)) {
}

FSMController::~FSMController() {
	// TODO Auto-generated destructor stub
}

Move
FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){

}

bool PillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPillPositions().size();
	if(last!=quedan && quedan%20==0){
		last =quedan;
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> PillTransition::getNextState(){
	return _next;
}


// detecta cuando el fantasma se vuelve comestible
FrightenedTransition::FrightenedTransition(std::shared_ptr<FSMState> next)
    : _next(next), lastEdible(false) {}
 
bool FrightenedTransition::isValid(const GameState& gs){
    bool nowEdible = gs.isGhostEdible(0);
    if(!lastEdible && nowEdible){
        lastEdible = nowEdible;
        return true;
    }
    lastEdible = nowEdible;
    return false;
}
std::shared_ptr<FSMState> FrightenedTransition::getNextState(){ return _next; }

// detecta cuando el fantasma deja de ser comestible
RecoveryTransition::RecoveryTransition(std::shared_ptr<FSMState> next)
	: _next(next) {}

bool RecoveryTransition::isValid(const GameState& gs){
	return !gs.isGhostEdible(0);
}

std::shared_ptr<FSMState> RecoveryTransition::getNextState(){ return _next; }

// Dispara despues de N segundos usando chrono
TimerTransition::TimerTransition(std::shared_ptr<FSMState> next, double seconds)
	: _next(next), _seconds(seconds), _running(false) {}
	void TimerTransition::Reset() {
		time_point_start = std::chrono::steady_clock::now();
		_running = true;
	}
bool TimerTransition::isValid(const GameState&){
    auto now = std::chrono::steady_clock::now(); 
    
    std::chrono::duration<double> diff = now - time_point_start;
    return diff.count() >= _seconds;
}
std::shared_ptr<FSMState> TimerTransition::getNextState(){ return _next; }

// se activa cuando quedan pocas pills (menos de 30)
FewPillsTransition::FewPillsTransition(std::shared_ptr<FSMState> next)
    : _next(next) {}

bool FewPillsTransition::isValid(const GameState& gs) {
    // Cuando quedan menos de 30 pills, activamos patrol
    int remainingPills = (int)gs.getMaze().getPillPositions().size();
    return remainingPills > 0 && remainingPills < 30;
}

std::shared_ptr<FSMState> FewPillsTransition::getNextState() { return _next; }

///////////////////////////////ChaseState///////////////////////////////////////
ChaseState::ChaseState(std::shared_ptr<Character> _character):FSMState(_character){

}
void ChaseState::onEnter(const GameState& ){
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}
Move ChaseState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min=euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
ChaseState::~ChaseState(){

}

//  Blinky huye a la esquina superior derecha
ScatterState::ScatterState(std::shared_ptr<Character> _character):FSMState(_character), corner({25, 0}){
	
}

void ScatterState::onEnter(const GameState&){
    std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move ScatterState::onUpdate(const GameState& game){
	// Moverse hacia la esquina fija, igual que chase pero apuntando a corner
    std::vector<Move> moves;
    const auto myPos = character->getPos();
 
    if(character->getDirection() == PASS){
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }
 
    float minDist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])), corner);
    int minI = 0;
    for(unsigned int i = 1; i < moves.size(); i++){
        auto dist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])), corner);
        if(dist < minDist){ minDist = dist; minI = i; }
    }
    return moves[minI];
}
ScatterState::~ScatterState(){
}

// Huir de Pacman eligiendo la casilla vecina mas lejos de pacman
FrightenedState::FrightenedState(std::shared_ptr<Character> _character)
    : FSMState(_character){}
	void FrightenedState::onEnter(const GameState&){
}
Move FrightenedState::onUpdate(const GameState& game){
    std::vector<Move> moves;
    const auto pacmanCoord = game.getMaze().getNodePos(game.getPacmanPos());
    const auto myPos = character->getPos();
 
    if(character->getDirection() == PASS){
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }
 
    float maxDist = -1.0f;
    int maxI = 0;
    for(unsigned int i = 0; i < moves.size(); i++){
        auto dist = euclid2(game.getMaze().getNodePos( game.getMaze().getNeighbour(myPos, moves[i])), pacmanCoord);
        if(dist > maxDist){ maxDist = dist; maxI = i; }
    }
    return moves[maxI];
}
FrightenedState::~FrightenedState(){}

// Estado nuevo que agregamos a Blinky. Blinky va a la esquina inferior izquierda del mapa por un momento.
PatrolState::PatrolState(std::shared_ptr<Character> _character)
    : FSMState(_character), patrolCorner({0, 30}) {}

void PatrolState::onEnter(const GameState&) {
    // cuando entramos al estado patrol, revertimos si estabamos asustados
    std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move PatrolState::onUpdate(const GameState& game) {
    // mismo algoritmo de scatter pero apuntando a la esquina inferior izquierda
    std::vector<Move> moves;
    const auto myPos = character->getPos();

    if(character->getDirection() == PASS){
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }

    float minDist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])), patrolCorner);
    int minI = 0;
    for(unsigned int i = 1; i < moves.size(); i++){
        auto dist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])), patrolCorner);
        if(dist < minDist){ minDist = dist; minI = i; }
    }
    return moves[minI];
}

PatrolState::~PatrolState() {}
 
NonFrightenedState:: NonFrightenedState(std::shared_ptr<Character> _character)
	: FSMState(_character) {
	chaseState   = std::make_shared<ChaseState>(_character);
    scatterState = std::make_shared<ScatterState>(_character);
    patrolState  = std::make_shared<PatrolState>(_character);
 
    chaseTimer   = std::make_shared<TimerTransition>(scatterState, 7.0);
    scatterTimer = std::make_shared<TimerTransition>(chaseState, 5.0);
    // Blinky patrulla 3 segundos y vuelve a perseguir
    patrolTimer  = std::make_shared<TimerTransition>(chaseState, 3.0);
    // Transicion: cuando quedan pocas pills, ir a patrullar
    fewPillsTrans = std::make_shared<FewPillsTransition>(patrolState);
 
    chaseState->addTransition(fewPillsTrans);  
    chaseState->addTransition(chaseTimer);     
    scatterState->addTransition(scatterTimer); 
    patrolState->addTransition(patrolTimer);   
 
    activeChild = chaseState;
}

void NonFrightenedState::onEnter(const GameState& gs){

    activeChild = chaseState;
    chaseTimer->Reset();
    activeChild->onEnter(gs);
}
Move NonFrightenedState::onUpdate(const GameState& gs){
    // Verificamos si el hijo activo quiere cambiar de estado
    auto t = activeChild->getActiveTransition(gs);
    if(t != nullptr){
        activeChild->onExit(gs);
        t->onTransition(gs);
        activeChild = t->getNextState();
        activeChild->onEnter(gs);
 
        // Reiniciamos el timer del nuevo hijo
        if(activeChild == chaseState)   chaseTimer->Reset();
        if(activeChild == scatterState) scatterTimer->Reset();
        if(activeChild == patrolState)  patrolTimer->Reset();
    }
    return activeChild->onUpdate(gs);
}
NonFrightenedState::~NonFrightenedState(){}

/////////////////////////////////////BlinkyStateMachine/////////////////////////////
BlinkyStateMachine::BlinkyStateMachine(std::shared_ptr<Character> _character):FiniteStateMachine(_character){
	auto nonFrightened = std::make_shared<NonFrightenedState>(_character);
    auto frightened    = std::make_shared<FrightenedState>(_character);
 
    
    nonFrightened->addTransition(std::make_shared<FrightenedTransition>(frightened));
 
    
    frightened->addTransition(std::make_shared<RecoveryTransition>(nonFrightened));
 
    states.push_back(nonFrightened);
    states.push_back(frightened);
 
    initialState = nonFrightened;
    activeState  = initialState;
}



Move BlinkyStateMachine::update(const GameState& gs){
	auto t=activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState=t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


BlinkyStateMachine::~BlinkyStateMachine(){

}

