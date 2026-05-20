#include "InkyController.h"
#include "BTGhostController.h"  
#include <iostream>
#include <limits>

// Distancia maxima para considerar que Blinky esta cerca de Inky
const float BLINKY_NEAR_DIST = 10.0f * 10.0f;


InkyController::InkyController(std::shared_ptr<Character> character)
    : Controller(character), root(std::make_shared<Selector>()) {

    // Rama 1: si es comestible  huir
    auto filter1 = std::make_shared<Filter>();
    filter1->addCondition(std::make_shared<InkyIsEdible>());
    filter1->addAction(std::make_shared<InkyFlee>());
    root->addChild(filter1);

    // Rama 2: si el timer lo dice  scatter a esquina
    auto filter2 = std::make_shared<Filter>();
    filter2->addCondition(std::make_shared<InkyTimeOut>());
    filter2->addAction(std::make_shared<InkyScatter>());
    root->addChild(filter2);

    // Rama 3: si Blinky esta cerca  coordinar y flanquear
    auto filter3 = std::make_shared<Filter>();
    filter3->addCondition(std::make_shared<BlinkyNearby>());
    filter3->addAction(std::make_shared<InkyCoChase>());
    root->addChild(filter3);

    // Rama 4: por defecto perseguir directo
    root->addChild(std::make_shared<InkyChase>());
}

InkyController::~InkyController() {}

Move InkyController::getMove(const GameState& gs) {
    Info::getInfo()->in_character = character;
    Info::getInfo()->in_gamestate = &gs;
    root->tick();
    return Info::getInfo()->out_move;
}

// chequea si Inky es comestible
Status InkyIsEdible::update() {
    auto character = Info::getInfo()->in_character;
    auto ghost = dynamic_cast<Ghost*>(character.get());
    if(ghost != nullptr && ghost->isEdible()){
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}

//  controla el tiempo de scatter
InkyTimeOut::InkyTimeOut() : Behavior() {
    lastTime = std::chrono::high_resolution_clock::now();
}

Status InkyTimeOut::update() {
    std::chrono::duration<float> timeStamp = std::chrono::high_resolution_clock::now() - lastTime;
    if((int)timeStamp.count() % 27 < 7){
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}

//  chequea si Blinky esta cerca de Inky
Status BlinkyNearby::update() {
    auto gs = Info::getInfo()->in_gamestate;
    auto character = Info::getInfo()->in_character;

    auto inkyPos = gs->getMaze().getNodePos(character->getPos());
    auto blinkyPos = gs->getMaze().getNodePos(gs->getGhostsPos(0));

    float dist = euclid2(inkyPos, blinkyPos);
    if(dist <= BLINKY_NEAR_DIST){
        return BH_SUCCESS; 
    }
    return BH_FAILURE;
}

// alejarse de Pacman eligiendo el vecino mas lejano
Status InkyFlee::update() {
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

//  ir hacia la esquina superior izquierda
InkyScatter::InkyScatter() : Behavior(), target({2, 0}) {}

Status InkyScatter::update() {
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

//  perseguir directo a Pacman 
Status InkyChase::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;
    auto target = gs->getMaze().getNodePos(gs->getPacmanPos());

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

//  flanquear a Pacman apuntando 2 pasos adelante de su posicion

Status InkyCoChase::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;

    auto pacmanPos = gs->getMaze().getNodePos(gs->getPacmanPos());
    int pacmanDir = gs->getPacmanDir();
    // Usamos offsets fijos segun la direccion 
    int offsetX = 0, offsetY = 0;
    const int OFFSET = 16;  
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
