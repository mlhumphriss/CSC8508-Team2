#include "MainMenu.h";

//using namespace NCL;

namespace NCL {
	namespace CSC8503 {

		class OverlayScreen : public PushdownState
		{
		public:

			OverlayScreen(OnAwakeFunction onAwake, OnUpdateFunction onUpdate) : PushdownState(onAwake, onUpdate) {};

			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				return PushdownState::OnUpdate(dt, newState);
			}
			void OnAwake() override
			{
				PushdownState::OnAwake();
			}
		};

		MainMenu::MainMenu(SetPauseGame setPauseFunc)
		{
			setPause = setPauseFunc;
			machine = new PushdownMachine(new OverlayScreen(
				[&]() -> void { this->OnStateAwake(); },
				[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
					return this->GameScreenOnUpdate(dt, newState);
				}
			));

			machine->PushStateToStack(new OverlayScreen(
				[&]() -> void { this->OnStateAwakePause(); },
				[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
					return this->IntroScreenOnUpdate(dt, newState);
				}
			));
		}

		void MainMenu::OnStateAwake()
		{
			setPause(false);
		}

	
		PushdownState::PushdownResult MainMenu::GameScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Game Screen", Vector2(5, 85));

			if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
				*newState = new OverlayScreen(
					[&]()-> void { this->OnStateAwakePause(); },
					[&](float dt, PushdownState** newState) -> PushdownState::PushdownResult {
						return this->PauseScreenOnUpdate(dt, newState);
					}
				);
				return PushdownState::PushdownResult::Push;
			}
			return PushdownState::PushdownResult::NoChange;
		}

		PushdownState::PushdownResult MainMenu::PauseScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Paused", Vector2(5, 85));

			if (Window::GetKeyboard()->KeyPressed(KeyCodes::P))
			{
				setPause(false);
				return PushdownState::PushdownResult::Pop;
			}
			return PushdownState::PushdownResult::NoChange;
		}


		PushdownState::PushdownResult MainMenu::IntroScreenOnUpdate(float dt, PushdownState** newState)
		{
			Debug::Print("Main Menu: C->Start as client, V->Start as server, B->Start Offline", Vector2(5, 85));
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
				setPause(false);
				return PushdownState::PushdownResult::Pop;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::V)) {
				setPause(false);
				return PushdownState::PushdownResult::Pop;
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::B)) {
				setPause(false);
				return PushdownState::PushdownResult::Pop;
			}
			return PushdownState::PushdownResult::NoChange;
		}

		MainMenu::~MainMenu() {
			delete activeController;
		}


		void MainMenu::Update(float dt)
		{
			if (!machine->Update(dt))
				return;
		}
	}
}