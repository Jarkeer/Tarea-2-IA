#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>
#include <random>


  
class InkyController: public Controller {
private:
    std::shared_ptr<Composite> root;
public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game) override;
};

class InkyIsEdible : public Behavior {
public:
    virtual Status update() override;
};

// tiempo de scatter 
class InkyTimeOut : public Behavior {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
    InkyTimeOut();
    virtual Status update() override;
};

// Blinky esta cerca de Inky?
class BlinkyNearby : public Behavior {
public:
    virtual Status update() override;
};

// huir de Pacman 
class InkyFlee : public Behavior {
public:
    virtual Status update() override;
};

// ir a esquina superior izquierda
class InkyScatter : public Behavior {
private:
    std::pair<int,int> target;
public:
    InkyScatter();
    virtual Status update() override;
};

//  perseguir a Pacman directamente
class InkyChase : public Behavior {
public:
    virtual Status update() override;
};

//  apuntar 2 nodos adelante de Pacman
class InkyCoChase : public Behavior {
public:
    virtual Status update() override;
};
