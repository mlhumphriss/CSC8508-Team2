#pragma once
#include "NavigationMesh.h"
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class BehaviourTree;
        class EnemyGameObject : public GameObject {
        public:
            EnemyGameObject();
            ~EnemyGameObject();
            virtual void Update(float dt);

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            BehaviourTree* stateMachine;
            NavigationMesh* navMesh;

            float counter;
        };
    }
}
