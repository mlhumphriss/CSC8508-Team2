#pragma once
#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "CollisionDetection.h"




namespace NCL {
    namespace CSC8503 {
        class EnemyGameObject : public GameObject {
        public:    

            typedef std::function<bool(Ray& r, RayCollision& closestCollision, bool closestObject)> RaycastToWorld; 
            typedef std::function<Vector3()> GetPlayerPos;

            EnemyGameObject(NavigationMesh* navMesh);
            ~EnemyGameObject();
            virtual void Update(float dt);

            void SetRay(RaycastToWorld rayHit){ this->rayHit = rayHit; }
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }
            void MoveAlongPath();

        protected:
 
            bool RayCastPlayer();
            void SetPath(Vector3 startPos, Vector3 endPos);

            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;


            BehaviourSequence* sequence = nullptr;
            NavigationMesh* navMesh = nullptr;

            NavigationPath outPath;
            BehaviourState state;

            const int wayPointsLength = 3;
   
            Vector3 wayPoints[3] = {
                Vector3(50, 22, -50),
                Vector3(-50, 22, 0),
                Vector3(0, 22, 50)
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
                        if (playerDis <= 100.0f) {
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

            BehaviourAction* chase = new BehaviourAction("Chase",
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
