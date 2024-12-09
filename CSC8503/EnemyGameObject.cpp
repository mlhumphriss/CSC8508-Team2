#include "EnemyGameObject.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

EnemyGameObject::EnemyGameObject()
{
    counter = 0.0f;
    sequence = new BehaviourSequence("Room Sequence");
    state = Ongoing;

    sequence->Reset();
    behaviourTimer = 0.0f;
}


EnemyGameObject::~EnemyGameObject() {
    delete sequence;
}

void EnemyGameObject::Update(float dt) {
    state = sequence->Execute(dt);
}

void EnemyGameObject::MoveLeft(float dt) {
    GetPhysicsObject()->AddForce({ -100, 0, 0 });
    counter += dt;
}

void EnemyGameObject::MoveRight(float dt) {
    GetPhysicsObject()->AddForce({ 100, 0, 0 });
    counter -= dt;
}
