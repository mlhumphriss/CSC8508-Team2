#include "EnemyGameObject.h"

using namespace NCL;
using namespace CSC8503;

void EnemyGameObject::SetPath(Vector3 startPos, Vector3 endPos)
{
    bool found = navMesh->FindPath(startPos, endPos, outPath);

    if (found) {
        navMesh->SmoothPath(outPath);
        outPathIndex = 0;
    }
}

EnemyGameObject::EnemyGameObject(NavigationMesh* navMesh)
{
	this->navMesh = navMesh;
    sequence = new BehaviourSequence("Path Sequence");
    sequence->AddChild(patrol);

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
    Vector3 playerToEnemy = enemyPos - playerPos;

    Ray ray = Ray(enemyPos, playerToEnemy);
    playerDis = Vector::Length(playerToEnemy);

    RayCollision closestCollision;

    if (rayHit(ray, closestCollision, true))
        return true;
    return false;
}

void EnemyGameObject::MoveAlongPath() {

}

void EnemyGameObject::Update(float dt) 
{
    state = sequence->Execute(dt);
}

