#pragma once

#include "GameObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "UpdateObject.h"

#include "NavMeshAgent.h"
#include "CollisionDetection.h"
#include "GameWorld.h"

namespace NCL {
    namespace CSC8503 {
        class Kitten : public NavMeshAgent, public UpdateObject {
        public:

            Kitten(NavigationMesh* navMesh, GameObject* swarm);
            ~Kitten();
            void SetSelected(bool state) { selected = state; }
            bool GetSelected() { return selected; }

            void Update(float dt) override
            {
                if (state != Ongoing) {
                    sequence->Reset();
                    state = Ongoing;
                }

                if (selected) {
                    state = sequence->Execute(dt);
                    MoveAlongPath();
                }
            }

            virtual void OnCollisionBegin(GameObject* otherObject) override {
                if (otherObject->GetTag() == Tags::CursorCast) {
                    selected = true;
                }
            }


        protected:

            BehaviourSequence* sequence = nullptr;
            BehaviourState state;
            GameObject* swarmCenter = nullptr;

            bool selected;


            BehaviourAction* idle = new BehaviourAction("Idle",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    if (state == Initialise)
                        state = Ongoing;
                    else if (state == Ongoing && selected)
                            return Success;
                    return state;
                }
            );

            BehaviourAction* goToSwarm = new BehaviourAction("GoToSwarm",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 swarmPos  = swarmCenter->GetTransform().GetPosition();

                    if (state == Initialise) {
                        SetPath(pos, swarmPos);
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (Vector::Length(pos - swarmPos) < 6.0f)
                        {
                            testNodes.clear();
                            return Success;
                        }
                        else
                            SetPath(pos, swarmPos);
                    }
                    return state;
                }
            );


            BehaviourAction* followSwarm = new BehaviourAction("FollowSwarm",
                [&](float dt, BehaviourState state) -> BehaviourState
                {
                    Vector3 pos = this->transform.GetPosition();
                    Vector3 swarmPos = swarmCenter->GetTransform().GetPosition();

                    if (state == Initialise) {
                        testNodes.clear();
                        state = Ongoing;
                    }
                    else if (state == Ongoing)
                    {
                        if (Vector::Length(pos - swarmPos) > 10.0f) {
                            testNodes.clear();
                            selected = false;
                            return Failure;
                        }
                    }
                    return state;
                }
            );
        };
    }
}