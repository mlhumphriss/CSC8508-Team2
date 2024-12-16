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
    Vector3 currentPos = this->GetTransform().GetPosition();

    for (auto obj : objects) {
        if (obj) 
        { 
            Vector3 v1 = rule1(obj, objects);
            Vector3 v2 = rule2(obj, objects);
            Vector3 v3 = rule3(obj, objects);

            obj->GetPhysicsObject()->AddForce(Vector::Normalise(v1 + v2 +v3) * 2.0f);

            auto pos = obj->GetTransform().GetPosition();
            auto dir = currentPos - pos;
            auto force = Vector::Normalise(dir);

            if (Vector::Length(dir) > 7)
                obj->GetPhysicsObject()->AddForce(force * 5.0f);

        }
    }
}

const float MIN_DISTANCE = 10.0f;

Vector3 Swarm::rule1(GameObject*& b, std::vector<GameObject*>& boids) {
    Vector3 perceived_center = this->GetTransform().GetPosition();

    int count = 0;
    for (auto& other : boids) {
        auto otherPos = other->GetTransform().GetPosition();
        if (&other != &b &&  (Vector::Length(b->GetTransform().GetPosition() - otherPos) < MIN_DISTANCE)) {
            perceived_center = perceived_center + otherPos;
            count++;
        }
    }

    if (count > 0) {
        perceived_center = perceived_center / static_cast<float>(count);
        return (perceived_center - b->GetTransform().GetPosition()) / 100.0f;
    }
    return Vector3(0, 0, 0);
}

Vector3 Swarm::rule2(GameObject*& b, std::vector<GameObject*>& boids) {
    Vector3 c = Vector3(0, 0, 0);
    for (auto& other : boids) {
        if (&other != &b && Vector::Length(b->GetTransform().GetPosition() - other->GetTransform().GetPosition()) < MIN_DISTANCE) {
            c = c - (other->GetTransform().GetPosition() - b->GetTransform().GetPosition());
        }
    }
    return c;
}

Vector3 Swarm::rule3(GameObject*& b, std::vector<GameObject*>& boids) {
    Vector3 perceived_velocity(0, 0, 0);
    int count = 0;
    for (auto& other : boids) {
        if (&other != &b && Vector::Length(b->GetTransform().GetPosition() - other->GetTransform().GetPosition()) < MIN_DISTANCE) {
            perceived_velocity = perceived_velocity + other->GetPhysicsObject()->GetLinearVelocity();
            count++;
        }
    }

    if (count > 0) {
        perceived_velocity = perceived_velocity / static_cast<float>(count);;
        return (perceived_velocity - b->GetTransform().GetPosition()) / 8.0f;
    }
    return Vector3(0, 0, 0);
}


void Swarm::Update(float dt)
{

    if (state != Ongoing) {
        sequence->Reset();
        state = Ongoing;
    }

    state = sequence->Execute(dt);
    DisplayPathfinding(Vector4(0, 1, 0, 1));
    MoveAlongPath();
    MoveObjectsAlongSwarm();
}

