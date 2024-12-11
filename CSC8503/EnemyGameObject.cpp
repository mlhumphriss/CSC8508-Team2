#include "EnemyGameObject.h"
#include "PhysicsObject.h"
#include "Ray.h"
#include "CollisionDetection.h"

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

   // Ray ray = CollisionDetection::BuildRayFromMouse();
    RayCollision closestCollision;
    Vector3 playerPos;

    /*if (world->Raycast(ray, closestCollision, true))
    {
        playerPos = closestCollision.collidedAt;
        return true;
    }
    */
    return false;
}

float EnemyGameObject::GetPlayerDistance()
{
    return 0.0f;
    /*if () {

    } else
        playerDis = FLT_MAX;*/
}

void EnemyGameObject::Update(float dt) 
{
    playerDis = GetPlayerDistance();
    state = sequence->Execute(dt);
}

