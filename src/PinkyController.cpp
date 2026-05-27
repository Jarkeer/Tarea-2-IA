#include "PinkyController.h"
#include "BTGhostController.h"  
#include "Ghost.h"
#include <iostream>
#include <limits>

PinkyController::PinkyController(std::shared_ptr<Character> character)
    : Controller(character), root(std::make_shared<Selector>()) {

    // Rama 1: si es comestible -> huir
    auto filter1 = std::make_shared<Filter>();
    filter1->addCondition(std::make_shared<PinkyIsEdible>());
    filter1->addAction(std::make_shared<PinkyFlee>());
    root->addChild(filter1);

    // Rama 2: si el timer dice scatter -> ir a esquina
    auto filter2 = std::make_shared<Filter>();
    filter2->addCondition(std::make_shared<PinkyTimeOut>());
    filter2->addAction(std::make_shared<PinkyScatter>());
    root->addChild(filter2);

    // Rama 3: si Pacman esta en pasillo -> interceptar
    auto filter3 = std::make_shared<Filter>();
    filter3->addCondition(std::make_shared<PacmanInHallway>());
    filter3->addAction(std::make_shared<PinkyIntercept>());
    root->addChild(filter3);

    // Rama 4: por defecto, emboscada 
    root->addChild(std::make_shared<PinkyAmbush>());
}

PinkyController::~PinkyController() {}

Move PinkyController::getMove(const GameState& gs) {
    Info::getInfo()->in_character = character;
    Info::getInfo()->in_gamestate = &gs;
    root->tick();
    return Info::getInfo()->out_move;
}

//  Pinky es ghost 2
Status PinkyIsEdible::update() {
    auto character = Info::getInfo()->in_character;
    auto ghost = dynamic_cast<Ghost*>(character.get());
    if(ghost != nullptr && ghost->isEdible()){
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}

// scatter timer 
PinkyTimeOut::PinkyTimeOut() : Behavior() {
    lastTime = std::chrono::high_resolution_clock::now();
}

Status PinkyTimeOut::update() {
    std::chrono::duration<float> timeStamp = std::chrono::high_resolution_clock::now() - lastTime;
    if((int)timeStamp.count() % 27 < 7){
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}

// detecta si el nodo de Pacman es un pasillo
Status PacmanInHallway::update() {
    auto gs = Info::getInfo()->in_gamestate;
    int pacPos = gs->getPacmanPos();
    // si el nodo de Pacman es un pasillo largo, retornar SUCCESS
    if(gs->getMaze().isHallway(pacPos)){
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}

// alejarse de Pacman eligiendo el vecino mas lejano
Status PinkyFlee::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;
    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());

    std::vector<Move> moves;
    if(character->getDirection() == PASS){
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    float maxDist = -1.0f;
    Move maxMove = PASS;
    for(auto move : moves){
        if(move == PASS) break;
        float dist = euclid2(pacmanPos,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if(dist > maxDist){ maxDist = dist; maxMove = move; }
    }
    Info::getInfo()->out_move = maxMove;
    return BH_SUCCESS;
}

//  ir a esquina superior derecha (diferente a la de Inky)
PinkyScatter::PinkyScatter() : Behavior(), target({23, 0}) {}

Status PinkyScatter::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;

    std::vector<Move> moves;
    if(character->getDirection() == PASS){
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    float minDist = std::numeric_limits<float>::max();
    Move minMove = PASS;
    for(auto move : moves){
        if(move == PASS) break;
        float dist = euclid2(target,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if(dist < minDist){ minDist = dist; minMove = move; }
    }
    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}

// apuntar 4 casillas adelante de donde va Pacman
Status PinkyAmbush::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;

    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
    int pacmanDir = gs->getPacmanDir();

    int offsetX = 0, offsetY = 0;
    const int OFFSET = 32;  
    if(pacmanDir == UP)    offsetY = -OFFSET;
    if(pacmanDir == DOWN)  offsetY =  OFFSET;
    if(pacmanDir == LEFT)  offsetX = -OFFSET;
    if(pacmanDir == RIGHT) offsetX =  OFFSET;

    std::pair<int,int> objetivo = {pacmanPos.first + offsetX, pacmanPos.second + offsetY};

    std::vector<Move> moves;
    if(character->getDirection() == PASS){
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    float minDist = std::numeric_limits<float>::max();
    Move minMove = PASS;
    for(auto move : moves){
        if(move == PASS) break;
        float dist = euclid2(objetivo,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if(dist < minDist){ minDist = dist; minMove = move; }
    }
    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}

// ir al frente del pasillo donde esta Pacman para bloquearlo
Status PinkyIntercept::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;

    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
    int pacmanDir = gs->getPacmanDir();

    int offsetX = 0, offsetY = 0;
    const int OFFSET = 48;
    if(pacmanDir == UP)    offsetY = -OFFSET;
    if(pacmanDir == DOWN)  offsetY =  OFFSET;
    if(pacmanDir == LEFT)  offsetX = -OFFSET;
    if(pacmanDir == RIGHT) offsetX =  OFFSET;

    std::pair<int,int> objetivo = {pacmanPos.first + offsetX, pacmanPos.second + offsetY};

    std::vector<Move> moves;
    if(character->getDirection() == PASS){
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    float minDist = std::numeric_limits<float>::max();
    Move minMove = PASS;
    for(auto move : moves){
        if(move == PASS) break;
        float dist = euclid2(objetivo,
            gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if(dist < minDist){ minDist = dist; minMove = move; }
    }
    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}
