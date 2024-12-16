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

            struct BoidRules {
                float minDistanceRule2 = 2.0f;
                float minDistanceRule3 = 0.5f;
                float forceMultiplier = 0.5f;
                float maxDistanceToCenter = 9.0f;

                float rule1Weight = 1.0f;
                float rule2Weight = 1.0f;
                float rule3Weight = 0.0f;
                int roundingPrecision = 1;
            };
            BoidRules ruleConfig;
            RaycastToWorld rayHit;
            GetPlayerPos getPlayerPos;

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;
            void MoveObjectsAlongSwarm();

            Vector3 rule1(GameObject*& b, std::vector<GameObject*>& boids);
            Vector3 rule2(GameObject*& b, std::vector<GameObject*>& boids);
            Vector3 rule3(GameObject*& b, std::vector<GameObject*>& boids);

            vector<GameObject*> objects;

            float separationRadius = 10.0f;
            float alignmentRadius = 20.0f;
            float cohesionRadius = 30.0f;

            float separationWeight = 3.0f;
            float alignmentWeight = 2.0f;
            float cohesionWeight = 2.0f;

            float maxForce = 20.0f;

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
