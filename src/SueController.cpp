#include "SueController.h"
#include "Ghost.h"
#include <iostream>


const float CLYDE_DISTANCE = 8.0f * 8.0f;  

// Transiciones

ClydeFarTransition::ClydeFarTransition(std::shared_ptr<FSMState> next)
    : _next(next) {}

bool ClydeFarTransition::isValid(const GameState& gs) {
    auto myPos = gs.getMaze().getNodePos(gs.getGhostsPos(3));
    auto pacPos = gs.getMaze().getNodePos(gs.getPacmanPos());
    float dist = euclid2(myPos, pacPos);
    return dist > CLYDE_DISTANCE;  
}

std::shared_ptr<FSMState> ClydeFarTransition::getNextState() { return _next; }


ClydeNearTransition::ClydeNearTransition(std::shared_ptr<FSMState> next)
    : _next(next) {}

bool ClydeNearTransition::isValid(const GameState& gs) {
    // Si esta muy cerca de Pacman, irse a su esquina
    auto myPos = gs.getMaze().getNodePos(gs.getGhostsPos(3));
    auto pacPos = gs.getMaze().getNodePos(gs.getPacmanPos());
    float dist = euclid2(myPos, pacPos);
    return dist <= CLYDE_DISTANCE;  
}

std::shared_ptr<FSMState> ClydeNearTransition::getNextState() { return _next; }


ClydeEdibleTransition::ClydeEdibleTransition(std::shared_ptr<FSMState> next)
    : _next(next), lastEdible(false) {}

bool ClydeEdibleTransition::isValid(const GameState& gs) {
    bool nowEdible = gs.isGhostEdible(3);
    if(!lastEdible && nowEdible) {
        lastEdible = nowEdible;
        return true;
    }
    lastEdible = nowEdible;
    return false;
}

std::shared_ptr<FSMState> ClydeEdibleTransition::getNextState() { return _next; }


ClydeNotEdibleTransition::ClydeNotEdibleTransition(std::shared_ptr<FSMState> next)
    : _next(next) {}

bool ClydeNotEdibleTransition::isValid(const GameState& gs) {
    return !gs.isGhostEdible(3);
}

std::shared_ptr<FSMState> ClydeNotEdibleTransition::getNextState() { return _next; }


// Estados //

// perseguir a Pacman moviendose al vecino mas cercano
ClydeChaseState::ClydeChaseState(std::shared_ptr<Character> _character)
    : FSMState(_character) {}

void ClydeChaseState::onEnter(const GameState&) {
    std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move ClydeChaseState::onUpdate(const GameState& game) {
    std::vector<Move> moves;
    const auto myPos = character->getPos();
    const auto pacmanCoord = game.getMaze().getNodePos(game.getPacmanPos());

    if(character->getDirection() == PASS){
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }

    float minDist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])), pacmanCoord);
    int minI = 0;
    for(unsigned int i = 1; i < moves.size(); i++){
        auto dist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])), pacmanCoord);
        if(dist < minDist){ minDist = dist; minI = i; }
    }
    return moves[minI];
}

ClydeChaseState::~ClydeChaseState() {}


// ir hacia la esquina inferior izquierda del mapa
ClydeWanderState::ClydeWanderState(std::shared_ptr<Character> _character)
    : FSMState(_character), corner({0, 30}) {}

void ClydeWanderState::onEnter(const GameState&) {
    std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move ClydeWanderState::onUpdate(const GameState& game) {
    std::vector<Move> moves;
    const auto myPos = character->getPos();

    if(character->getDirection() == PASS){
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }

    // Ir hacia la esquina inferior izquierda
    float minDist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])), corner);
    int minI = 0;
    for(unsigned int i = 1; i < moves.size(); i++){
        auto dist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])), corner);
        if(dist < minDist){ minDist = dist; minI = i; }
    }
    return moves[minI];
}

ClydeWanderState::~ClydeWanderState() {}


// huir de Pacman cuando come power pill
ClydeFrightenedState::ClydeFrightenedState(std::shared_ptr<Character> _character)
    : FSMState(_character) {}

void ClydeFrightenedState::onEnter(const GameState&) {}

Move ClydeFrightenedState::onUpdate(const GameState& game) {
    std::vector<Move> moves;
    const auto myPos = character->getPos();
    const auto pacmanCoord = game.getMaze().getNodePos(game.getPacmanPos());

    if(character->getDirection() == PASS){
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }

    // elegir la casilla mas alejada de Pacman
    float maxDist = -1.0f;
    int maxI = 0;
    for(unsigned int i = 0; i < moves.size(); i++){
        auto dist = euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])), pacmanCoord);
        if(dist > maxDist){ maxDist = dist; maxI = i; }
    }
    return moves[maxI];
}

ClydeFrightenedState::~ClydeFrightenedState() {}


// ClydeStateMachine 

ClydeStateMachine::ClydeStateMachine(std::shared_ptr<Character> _character)
    : FiniteStateMachine(_character) {

    auto chaseState     = std::make_shared<ClydeChaseState>(_character);
    auto wanderState    = std::make_shared<ClydeWanderState>(_character);
    auto frightenedState = std::make_shared<ClydeFrightenedState>(_character);

    //  si Pacman esta cerca -> ir a esquina 
    chaseState->addTransition(std::make_shared<ClydeEdibleTransition>(frightenedState));
    chaseState->addTransition(std::make_shared<ClydeNearTransition>(wanderState));

    // si Pacman se fue lejos -> volver a perseguir
    wanderState->addTransition(std::make_shared<ClydeEdibleTransition>(frightenedState));
    wanderState->addTransition(std::make_shared<ClydeFarTransition>(chaseState));

    //  cuando ya no es comestible -> volver a chas
    frightenedState->addTransition(std::make_shared<ClydeNotEdibleTransition>(chaseState));

    states.push_back(chaseState);
    states.push_back(wanderState);
    states.push_back(frightenedState);

    initialState = chaseState;
    activeState  = initialState;
}

Move ClydeStateMachine::update(const GameState& gs) {
    auto t = activeState->getActiveTransition(gs);
    if(t != nullptr){
        activeState->onExit(gs);
        t->onTransition(gs);
        activeState = t->getNextState();
        activeState->onEnter(gs);
    }
    return activeState->onUpdate(gs);
}

ClydeStateMachine::~ClydeStateMachine() {}


// SueController

SueController::SueController(std::shared_ptr<Character> character)
    : Controller(character),
      fsm(std::make_shared<ClydeStateMachine>(character)) {
}

SueController::~SueController() {}

Move SueController::getMove(const GameState& game) {
    return fsm->update(game);
}
