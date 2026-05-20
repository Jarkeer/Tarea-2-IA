/*
 * FSMController.h
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#ifndef FSMCONTROLLER_H_
#define FSMCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class BlinkyStateMachine;

class FSMController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<BlinkyStateMachine> fsm;
public:
	FSMController(std::shared_ptr<Character> character);
	virtual ~FSMController();
	virtual Move getMove(const GameState& game)override;
};

class FrightenedTransition:public FSMTransition{
	std::shared_ptr<FSMState> _next;
	bool lastEdible;
public:
	FrightenedTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

class RecoveryTransition:public FSMTransition{
	std::shared_ptr<FSMState> _next;
public:
	RecoveryTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

class TimerTransition : public FSMTransition {
	std::shared_ptr<FSMState> _next;
	double _seconds;
	std::chrono::steady_clock::time_point time_point_start;
	bool _running;
public:
	TimerTransition(std::shared_ptr<FSMState> next, double seconds);
	void Reset();
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState() override;
};

class PillTransition:public FSMTransition{
	int last;
	std::shared_ptr<FSMState> _next;
public:
	PillTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

// Transicion: pocas pills quedan en el mapa -> activar Patrol
class FewPillsTransition : public FSMTransition {
	std::shared_ptr<FSMState> _next;
public:
	FewPillsTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

// ESTADOS

class ChaseState:public FSMState{

public:
	ChaseState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ChaseState();

};

class ScatterState:public FSMState{
	std::pair<int,int> corner;

public: 
	ScatterState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
    Move onUpdate(const GameState& gs) override;
    ~ScatterState();
};

class FrightenedState:public FSMState{
	public:
	FrightenedState(std::shared_ptr<Character> _character);
    void onEnter(const GameState& gs) override;
    Move onUpdate(const GameState& gs) override;
    ~FrightenedState();
};

// Estado Patrol: Blinky va a la esquina inferior izquierda por un momento
// Es la modificacion que le agregamos a Blinky para que no solo persiga siempre
class PatrolState : public FSMState {
	std::pair<int,int> patrolCorner;
public:
	PatrolState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;	
	Move onUpdate(const GameState& gs) override;
	~PatrolState();
};

class NonFrightenedState:public FSMState{
	std::shared_ptr<FSMState> activeChild;   
    std::shared_ptr<ChaseState>   chaseState;
    std::shared_ptr<ScatterState> scatterState;
    std::shared_ptr<PatrolState>  patrolState;
    std::shared_ptr<TimerTransition> chaseTimer;
    std::shared_ptr<TimerTransition> scatterTimer;
    std::shared_ptr<TimerTransition> patrolTimer;
    std::shared_ptr<FewPillsTransition> fewPillsTrans;
public:
    NonFrightenedState(std::shared_ptr<Character> _character);
    void onEnter(const GameState& gs) override;
    Move onUpdate(const GameState& gs) override;
    ~NonFrightenedState();
};

class BlinkyStateMachine: public FiniteStateMachine{

public:
	BlinkyStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~BlinkyStateMachine();

};
#endif /* FSMCONTROLLER_H_ */
