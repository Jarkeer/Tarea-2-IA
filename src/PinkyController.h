#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>

class PinkyController: public Controller {
private:
    std::shared_ptr<Composite> root;
public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState& game) override;
};

// Pinky es comestible
class PinkyIsEdible : public Behavior {
public:
    virtual Status update() override;
};

// timer de scatter activo
class PinkyTimeOut : public Behavior {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
    PinkyTimeOut();
    virtual Status update() override;
};

// Pacman esta en un pasillo?
class PacmanInHallway : public Behavior {
public:
    virtual Status update() override;
};

// huir de Pacman
class PinkyFlee : public Behavior {
public:
    virtual Status update() override;
};

// scatter a esquina superior derecha
class PinkyScatter : public Behavior {
private:
    std::pair<int,int> target;
public:
    PinkyScatter();
    virtual Status update() override;
};

// apuntar 4 casillas adelante de Pacman
class PinkyAmbush : public Behavior {
public:
    virtual Status update() override;
};

// interceptar a Pacman en el pasillo yendo al frente del pasillo
class PinkyIntercept : public Behavior {
public:
    virtual Status update() override;
};
