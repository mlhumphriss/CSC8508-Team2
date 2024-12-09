#pragma once
#include "PushdownMachine.h"
#include "PushdownState.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "TutorialGame.h"

#include "PushdownMachine.h"
#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"



class MainMenu {

public:
	MainMenu();
	~MainMenu();
	void Update(float dt);
	bool CheckUpdateGame();

protected:
	bool isPaused;
	PushdownMachine* machine = nullptr;
	const Controller* activeController = nullptr;



};
