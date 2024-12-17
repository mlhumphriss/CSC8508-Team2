#pragma once
#include "PhysicsObject.h"
#include "Ray.h"
#include "UpdateObject.h"
//#include "Controller.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8503 {
        class PlayerGameObject : public UpdateObject, public GameObject {
        public:

            PlayerGameObject();
            ~PlayerGameObject();

            void Update(float dt) override
            {

            }

            virtual void OnCollisionBegin(GameObject* otherObject) override {
                
            }


        protected:
            // Controller* playerController;

        };
    }
}