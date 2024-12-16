#include "Swarm.h"


using namespace NCL;
using namespace CSC8503;


Swarm::Swarm(NavigationMesh* navMesh) : NavMeshAgent(navMesh)
{
    sequence = new BehaviourSequence("Swarm Sequence");
    sequence->AddChild(chase);
    objects = vector<GameObject*>();
    state = Ongoing;
    sequence->Reset();
}

Swarm::~Swarm() {
    delete sequence;
}

void Swarm::MoveObjectsAlongSwarm()
{
    for (auto obj : objects) {
        if (obj) 
        { 
            Vector3 currentPos = obj->GetTransform().GetPosition();
            Vector3 swarmOffset = CalculateSwarmOffset(obj);
            Vector3 newPos = currentPos + swarmOffset;
            obj->GetTransform().SetPosition(newPos);
        }
    }
}

Vector3 Swarm::CalculateSwarmOffset(GameObject* obj)
{
    Vector3 separation = Vector3(0, 0, 0);
    Vector3 alignment = Vector3(0, 0, 0);
    Vector3 cohesion = Vector3(0, 0, 0);

    int neighborCount = 0;

    Vector3 currentPos = obj->GetTransform().GetPosition();
    Vector3 currentVelocity = obj->GetPhysicsObject()->GetLinearVelocity();

    for (auto otherObj : objects) {
        if (otherObj && otherObj != obj) {
            Vector3 otherPos = otherObj->GetTransform().GetPosition();
            Vector3 otherVelocity = otherObj->GetPhysicsObject()->GetLinearVelocity();

            float distance = Vector::Length(currentPos - otherPos);

            if (distance < separationRadius)
                separation += (currentPos - otherPos) / distance;

            if (distance < alignmentRadius) 
                alignment += otherVelocity;

            if (distance < cohesionRadius) {
                cohesion += otherPos;
                neighborCount++;
            }
        }
    }

    if (neighborCount > 0) {
        alignment = alignment / static_cast<float>(neighborCount);
        cohesion = (cohesion / static_cast<float>(neighborCount)) - currentPos;
    }

    Vector3 offset = (separation * separationWeight) +  (alignment * alignmentWeight) + (cohesion * cohesionWeight);

    if (Vector::Length(offset) > maxForce) 
        offset = Vector::Normalise(offset) * maxForce;

    return offset;
}

void Swarm::Update(float dt)
{

    if (state != Ongoing) {
        sequence->Reset();
        state = Ongoing;
    }

    state = sequence->Execute(dt);
    DisplayPathfinding();
    MoveAlongPath();
}

