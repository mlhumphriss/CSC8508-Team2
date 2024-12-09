#include "Controller.h"
#include "MainMenu.h"
#include "Window.h"
using namespace NCL;

class PauseScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) {
			return PushdownResult::Pop;
		}		
		if (Window::GetKeyboard()->KeyDown(KeyCodes::O))
		{
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override 
	{
	}
};

class GameScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		Debug::Print("Paused", Vector2(0, 0));

		if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) 
		{
			*newState = new PauseScreen();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override 
	{
		
	}
};

class IntroScreen : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override 
	{
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			*newState = new GameScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override
	{
		//isPaused = true;
	}
};

MainMenu::MainMenu() {
	isPaused = true;
	machine = new PushdownMachine(new IntroScreen());
}

MainMenu::~MainMenu() {
	delete activeController;
}

bool MainMenu::CheckUpdateGame() {
	return !isPaused;
}

void MainMenu::Update(float dt)
{
	if (!activeController) 
		return;
	auto yAxis = activeController->GetNamedAxis("UpDown");

	if (!machine->Update(dt)) 
		return;
}