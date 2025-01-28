#pragma once
#include "PhysicsObject.h"
#include "Ray.h"
#include "UpdateObject.h"
#include "CollisionDetection.h"



namespace NCL {
	namespace CSC8508 {
		class VictoryPlatform : public UpdateObject, public GameObject {
		public:

				typedef std::function<void(bool hasWon)> EndGame;

				VictoryPlatform(EndGame endGame)
				{
					this->SetLayerID(Layers::Ignore_Collisions);
					this->endGame = endGame;
				}
				~VictoryPlatform() {}

			protected:
				int kittensCounted;
				EndGame endGame;

				void Update(float dt) override
				{						
					if (kittensCounted >= 15)
						endGame(true);
					kittensCounted = 0;
				}

				virtual void OnCollisionBegin(GameObject* otherObject) override {
					if (otherObject->GetTag() == Tags::Kitten) {
						kittensCounted++;

					}
				}
		};
	}
}
