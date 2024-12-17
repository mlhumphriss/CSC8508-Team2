#pragma once
#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "NavMeshAgent.h"

#include "CollisionDetection.h"

#include "GameWorld.h"
#include "UpdateObject.h"


namespace NCL {
    namespace CSC8503 {
        class EnemyGameObject : public NavMeshAgent, public UpdateObject {
        public:    

            typedef std::function<bool(Ray& r, RayCollision& closestCollision, bool closestObject)> RaycastToWorld; 
            typedef std::function<Vector3()> GetPlayerPos;

            EnemyGameObject(NavigationMesh* navMesh);
            ~EnemyGameObject();

            void SetRay(RaycastToWorld rayHit){ this->rayHit = rayHit; }
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }

            void Update(float dt) override 
            {
                if (state != Ongoing) {
                    sequence->Reset();
                    state = Ongoing;
                }

                state = sequence->Execute(dt);
                DisplayPathfinding(Vector4(0, 0, 1, 1));
                MoveAlongPath();
                this->GetPhysicsObject()->RotateTowardsVelocity();
            }

        protected:
 
            bool RayCastPlayer();

            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;

            const int wayPointsLength = 4;
   
            Vector3 wayPoints[4] = { 
                Vector3(25, 100, 30),
                Vector3(50, 22, -50),
                Vector3(-50, 22, 0),
                Vector3(25, 22, 50)
            };

            float playerDis = 0.0f;
            const float yOffSet = 3.0f; 
            bool playerVisible;
            int wayPointIndex = 0;



            BehaviourAction* patrol = new BehaviourAction("Patrol",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise)
                    {                         
                        SetPath(pos, wayPoints[wayPointIndex]);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (!RayCastPlayer())
                        {
                            wayPointIndex = 0;
                            return Success;     
                        }

                        if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset)
                            wayPointIndex >= wayPointsLength ? wayPointIndex = 0 : wayPointIndex++;      
                        
                        pos.y += yOffSet;
                        playerPos.y += yOffSet;
                        
                        if (RayCastPlayer())        
                            Debug::DrawLine(pos, playerPos, Vector4(1, 0, 0, 1));
                        else 
                            Debug::DrawLine(pos, playerPos, Vector4(0, 1, 0, 1));

                        SetPath(pos, wayPoints[wayPointIndex]);

                    }
                    return state;
                }
            );

            BehaviourAction* chase = new BehaviourAction("Chase",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 playerPos = getPlayerPos();

                    if (state == Initialise)
                    {
                        SetPath(pos, playerPos);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (RayCastPlayer()) {
                            if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset) 
                                return  Failure;
                        }
                        else 
                            SetPath(pos, playerPos);
                    }
                    return state;
                }
            );
        };
    }
}
