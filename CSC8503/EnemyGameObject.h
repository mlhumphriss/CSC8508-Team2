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
            EnemyGameObject(NavigationMesh* navMesh);
            ~EnemyGameObject();
            virtual void Update(float dt);

        protected:
 

            bool RayCastPlayer();
            float GetPlayerDistance();
            void SetPath(Vector3 startPos, Vector3 endPos);

            BehaviourSequence* sequence = nullptr;
            NavigationMesh* navMesh = nullptr;

            NavigationPath outPath;
            BehaviourState state;

            const int wayPointsLength = 3;
   
            Vector3 wayPoints[3] = {
                Vector3(1, 1, 1),
                Vector3(2, 2, 2),
                Vector3(3, 3, 3)
            };

            float playerDis = 0.0f;

            int wayPointIndex = 0;
            int outPathIndex = 0;

            float minWayPointDistanceOffset = 10;

            BehaviourAction* patrol = new BehaviourAction("Patrol", 
                [&](float dt, BehaviourState state) -> BehaviourState 
                {
                    Vector3 pos = this->transform.GetPosition();
                    if (state == Initialise)
                    {   
                        SetPath(pos, wayPoints[wayPointIndex]);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (playerDis <= 0.0f) {
                            wayPointIndex = 0;
                            return Success;
                        }
                        else if (Vector::Length(pos - wayPoints[wayPointIndex]) < minWayPointDistanceOffset) 
                        {
                            wayPointIndex >= wayPointsLength ? wayPointIndex = 0 : wayPointIndex++;
                            SetPath(pos, wayPoints[wayPointIndex]);
                        }
                    }
                    return state; 
                }
            );
        };
    }
}
