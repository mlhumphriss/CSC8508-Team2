#include "Kitten.h"
using namespace NCL;
using namespace CSC8503;


Kitten::Kitten(NavigationMesh* navMesh, GameObject* swarm) : NavMeshAgent(navMesh)
{
    sequence = new BehaviourSequence("Kitten Sequence");
    sequence->AddChild(idle);
    sequence->AddChild(goToSwarm);
    sequence->AddChild(followSwarm);

    swarmCenter = swarm;
    selected = false;

    state = Ongoing;
    sequence->Reset();
}

Kitten::~Kitten() {
    delete sequence;
}


