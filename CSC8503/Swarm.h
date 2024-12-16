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

namespace NCL {
    namespace CSC8503 {
        class Swarm : public NavMeshAgent {
        public:

            typedef std::function<bool(Ray& r, RayCollision& closestCollision, bool closestObject)> RaycastToWorld;
            typedef std::function<Vector3()> GetPlayerPos;

            Swarm(NavigationMesh* navMesh);
            ~Swarm();
            virtual void Update(float dt);
            void SetGetPlayer(GetPlayerPos getPlayerPos) { this->getPlayerPos = getPlayerPos; }

            void AddObjectToSwarm(GameObject* object) { objects.push_back(object); }

            void RemoveObjectFromSwarm(GameObject* object) {
                objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());
            }

        protected:

            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;
            void MoveObjectsAlongSwarm();
            Vector3 CalculateSwarmOffset(GameObject* obj);

            vector<GameObject*> objects;
            float playerDis = 0.0f;


            float separationRadius = 5.0f;
            float alignmentRadius = 10.0f;
            float cohesionRadius = 15.0f;

            float separationWeight = 1.5f;
            float alignmentWeight = 1.0f;
            float cohesionWeight = 1.0f;

            float maxForce = 2.0f;

            const float yOffSet = 3.0f;
            bool playerVisible;

            BehaviourAction* chase = new BehaviourAction("FollowPlayer",
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
                        if (Vector::Length(pos - testNodes[0]) < minWayPointDistanceOffset) {
                            std::cout << "failure" << std::endl;
                            return Success;
                        }
                        else {
                            SetPath(pos, playerPos);
                        }
                    }
                    return state;
                }
            );
        };
    }
}
