
#pragma once
#include "PhysicsObject.h"
#include "UpdateObject.h"
#include "CollisionDetection.h"
#include "StateMachine.h"
#include "State.h"

#include "StateTransition.h"

namespace NCL {
	namespace CSC8508 {
		class CollectMe : public GameObject, public UpdateObject {
		public:

			CollectMe(float points)
			{
				this->SetTag(Tags::Collect);
				this->points = points;

				testMachine = new StateMachine();
				stateAB = new StateTransition(A, B, [&](void)-> bool {return data > 4; });
				stateBA = new StateTransition(B, A, [&](void)-> bool {return data < 1; });

				testMachine->AddState(A);
				testMachine->AddState(B);
				testMachine->AddTransition(stateAB);
				testMachine->AddTransition(stateBA);

			}

			~CollectMe() {
				delete testMachine;
				delete stateAB;
				delete stateBA;
			}


			void Update(float dt) override
			{
				testMachine->Update(dt);
				this->GetTransform().SetScale(Vector3(1, 1, 1) * data);
			}

			bool IsCollected() { return collected; }
			void SetCollected(bool state) {
				collected = state;
			}
			float GetPoints() { return points; }

		protected:
			bool collected = false;
			float points;
			float data = 0;
			StateMachine* testMachine;
			StateTransition* stateAB;
			StateTransition* stateBA;


			State* A = new State([&](float dt) -> void {
				data += 0.01f;
				});

			State* B = new State([&](float dt) -> void {
				data -= 0.01f;
				});
		};
	}
}
