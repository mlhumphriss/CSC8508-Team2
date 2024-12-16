#include "EnemyGameObject.h"


using namespace NCL;
using namespace CSC8503;


EnemyGameObject::EnemyGameObject(NavigationMesh* navMesh) : NavMeshAgent(navMesh)
{
    sequence = new BehaviourSequence("Path Sequence");

    sequence->AddChild(patrol);
    sequence->AddChild(chase);

    state = Ongoing;
    sequence->Reset();
}

EnemyGameObject::~EnemyGameObject() {
    delete sequence;
}

bool EnemyGameObject::RayCastPlayer() 
{
    Vector3 playerPos = getPlayerPos();
    Vector3 enemyPos = this->GetTransform().GetPosition();
    enemyPos.y += yOffSet;
    playerPos.y += yOffSet;

    Vector3 playerToEnemy =  playerPos - enemyPos;

    Ray ray = Ray(enemyPos, playerToEnemy);
    playerDis = Vector::Length(playerToEnemy);

    RayCollision closestCollision;

    if (rayHit(ray, closestCollision, true))
        return true;
    return false;
}


void EnemyGameObject::Update(float dt) 
{

    if (state != Ongoing) {
        sequence->Reset();
        state = Ongoing;
    }

    state = sequence->Execute(dt);
    DisplayPathfinding(Vector4(0, 0, 1, 1));
    MoveAlongPath();
}

