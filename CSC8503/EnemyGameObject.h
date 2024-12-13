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


#include "GameWorld.h"

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
            void DisplayPathfinding();
            void SetPath(Vector3 startPos, Vector3 endPos);

            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;


            BehaviourSequence* sequence = nullptr;
            NavigationMesh* navMesh = nullptr;
            vector<Vector3> testNodes;

            NavigationPath outPath;
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
            int outPathIndex = 0;

            float minWayPointDistanceOffset = 2;

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
                        {
                            wayPointIndex >= wayPointsLength ? wayPointIndex = 0 : wayPointIndex++;
                            SetPath(pos, wayPoints[wayPointIndex]);
                        }           
                        
                        pos.y += yOffSet;
                        playerPos.y += yOffSet;
                        
                        if (RayCastPlayer())
                        {                   
                            //std::cout << "spotted" << std::endl;
                            // wayPointIndex = 0;
                            //return Success;     
                            Debug::DrawLine(pos, playerPos, Vector4(1, 0, 0, 1));
                        }
                        else {
                            Debug::DrawLine(pos, playerPos, Vector4(0, 1, 0, 1));
                        }
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
                            if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset) {
                                std::cout << "failure" << std::endl;
                                return  Failure;
                            }
                        }
                        else {
                            std::cout << "In sight" << std::endl;
                            SetPath(pos, playerPos);
                        }
                    }
                    return state;
                }
            );
        };
    }
}
