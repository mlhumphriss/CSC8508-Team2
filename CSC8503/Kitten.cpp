#include "Kitten.h"
using namespace NCL;
using namespace CSC8503;


Kitten::Kitten(NavigationMesh* navMesh, GameObject* swarm) : NavMeshAgent(navMesh)
{
    sequence = new BehaviourSequence("Kitten Sequence");
    sequence->AddChild(idle);
    sequence->AddChild(goToSwarm);
    sequence->AddChild(followSwarm);
    speed = 7.5f;
    swarmCenter = swarm;
    selected = false;

    state = Ongoing;
    sequence->Reset();
}

Kitten::~Kitten() {
    delete sequence;
}

void Kitten::ThrowSelf(Vector3 dir) 
{
    auto transform = this->GetTransform();
    auto pos = transform.GetPosition();
    pos.y += 5.0f;   
    
    selected = false;

    transform.SetPosition(pos);
    dir.y += 0.7f;
    this->GetPhysicsObject()->AddForce(dir * 75.0f);

}


