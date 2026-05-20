#pragma once

#include "Controller.h"
#include "FSM.h"
#include <chrono>

//  Pacman esta lejos de Clyde 
class ClydeFarTransition : public FSMTransition {
    std::shared_ptr<FSMState> _next;
public:
    ClydeFarTransition(std::shared_ptr<FSMState> next);
    bool isValid(const GameState& gs) override;
    std::shared_ptr<FSMState> getNextState() override;
};

// Clyde esta cerca de Clyde 
class ClydeNearTransition : public FSMTransition {
    std::shared_ptr<FSMState> _next;
public:
    ClydeNearTransition(std::shared_ptr<FSMState> next);
    bool isValid(const GameState& gs) override;
    std::shared_ptr<FSMState> getNextState() override;
};

// el fantasma  se vuelve comestible
class ClydeEdibleTransition : public FSMTransition {
    std::shared_ptr<FSMState> _next;
    bool lastEdible;
public:
    ClydeEdibleTransition(std::shared_ptr<FSMState> next);
    bool isValid(const GameState& gs) override;
    std::shared_ptr<FSMState> getNextState() override;
};

//  el fantasma deja de ser comestible
class ClydeNotEdibleTransition : public FSMTransition {
    std::shared_ptr<FSMState> _next;
public:
    ClydeNotEdibleTransition(std::shared_ptr<FSMState> next);
    bool isValid(const GameState& gs) override;
    std::shared_ptr<FSMState> getNextState() override;
};

//  perseguir a Pacman directamente
class ClydeChaseState : public FSMState {
public:
    ClydeChaseState(std::shared_ptr<Character> _character);
    void onEnter(const GameState& gs) override;
    Move onUpdate(const GameState& gs) override;
    ~ClydeChaseState();
};

// ir a su esquina cuando pacman esta cerca
class ClydeWanderState : public FSMState {
    std::pair<int,int> corner;
public:
    ClydeWanderState(std::shared_ptr<Character> _character);
    void onEnter(const GameState& gs) override;
    Move onUpdate(const GameState& gs) override;
    ~ClydeWanderState();
};

// huir de Pacman
class ClydeFrightenedState : public FSMState {
public:
    ClydeFrightenedState(std::shared_ptr<Character> _character);
    void onEnter(const GameState& gs) override;
    Move onUpdate(const GameState& gs) override;
    ~ClydeFrightenedState();
};

// La FSM de Clyde
class ClydeStateMachine : public FiniteStateMachine {
public:
    ClydeStateMachine(std::shared_ptr<Character> _character);
    Move update(const GameState& gs) override;
    ~ClydeStateMachine();
};

class SueController: public Controller {
    std::shared_ptr<ClydeStateMachine> fsm;
public:
	SueController(std::shared_ptr<Character> character);
	virtual ~SueController();
	virtual Move getMove(const GameState& game) override;
};
