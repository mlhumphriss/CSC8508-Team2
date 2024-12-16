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
        class NavMeshAgent : public GameObject {
        public:
            NavMeshAgent(NavigationMesh* navMesh)
            {
                this->navMesh = navMesh;
            }

            //~NavMeshAgent();

            void MoveAlongPath()
            {
                Vector3 pos = this->transform.GetPosition();

                if (outPathIndex < 0)
                    return;

                if (testNodes.size() == 0)
                    return;

                if (Vector::Length(pos - testNodes[outPathIndex]) < minWayPointDistanceOffset)
                    outPathIndex--;

                if (outPathIndex < 0)
                    return;

                Vector3 target = testNodes[outPathIndex];
                Vector3 enemyPos = this->GetTransform().GetPosition();

                Vector3 dir = target - enemyPos;

                dir = Vector::Normalise(dir);
                dir.y += 0.2f;

               // this->transform.SetPosition(pos += (dir * 0.5f));
                this->GetPhysicsObject()->SetLinearVelocity(dir * 5.0f);
            }


        protected:

            void DisplayPathfinding(Vector4 colour)
            {
                for (int i = 1; i < testNodes.size(); ++i) {
                    Vector3 a = testNodes[i - 1];
                    Vector3 b = testNodes[i];

                    Debug::DrawLine(a, b, colour);
                }
            }

            bool ExistingPath(NavigationPath lastOutPath, NavigationPath outPath) {
                NavigationPath tempOutPath = outPath;
                bool pathsEqual = true;

                Vector3 lastPos, outPos;

                while (lastOutPath.PopWaypoint(lastPos) && tempOutPath.PopWaypoint(outPos)) {
                    if (lastPos.x != outPos.x || lastPos.y != outPos.y || lastPos.z != outPos.z) {
                        pathsEqual = false;
                        break;
                    }
                }
                if (pathsEqual && (lastOutPath.PopWaypoint(lastPos) || tempOutPath.PopWaypoint(outPos)))
                    pathsEqual = false;

                if (pathsEqual)
                    return true;

                return false;

            }
            void SetPath(Vector3 startPos, Vector3 endPos)
            {
                NavigationPath lastOutPath = outPath;
                outPath.clear();

                bool found = navMesh->FindPath(startPos, endPos, outPath);
                //navMesh->SmoothPath(outPath);

                if (ExistingPath(lastOutPath, outPath))
                    return;

                Vector3 pos;
                testNodes.clear();

                std::stack<Vector3> tempStack;

                // Done twice as smoothpath would invert the list
                while (outPath.PopWaypoint(pos)) {
                    tempStack.push(pos);
                }

                while (!tempStack.empty()) {
                    testNodes.push_back(tempStack.top());
                    tempStack.pop();
                }

                if (testNodes.size() >= 2 &&
                    (Vector::Length(this->GetTransform().GetPosition() - testNodes[testNodes.size() - 2])
                        < Vector::Length(testNodes[testNodes.size() - 2] - testNodes[testNodes.size() - 1])))

                    outPathIndex = testNodes.size() - 2;
                else
                    outPathIndex = testNodes.size() - 1;
            }


            BehaviourSequence* sequence = nullptr;
            NavigationMesh* navMesh = nullptr;
            vector<Vector3> testNodes;

            NavigationPath outPath;
            BehaviourState state;

            int outPathIndex = 0;
            float minWayPointDistanceOffset = 2;

        };
    }
}
