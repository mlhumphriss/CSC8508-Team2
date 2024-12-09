#pragma once
#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"


namespace NCL {
    namespace CSC8503 {
        class EnemyGameObject : public GameObject {
        public:
            EnemyGameObject();
            ~EnemyGameObject();
            virtual void Update(float dt);

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            BehaviourSequence* sequence;
            NavigationMesh* navMesh;
            BehaviourState state;

            float behaviourTimer = 0.0f;
            float counter;
        };
    }
}
