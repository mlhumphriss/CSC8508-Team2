#include "EnemyGameObject.h"


using namespace NCL;
using namespace CSC8503;


void EnemyGameObject::DisplayPathfinding()
{
    for (int i = 1; i < testNodes.size(); ++i) {
        Vector3 a = testNodes[i - 1];
        Vector3 b = testNodes[i];

        Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
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

void EnemyGameObject::SetPath(Vector3 startPos, Vector3 endPos)
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

EnemyGameObject::EnemyGameObject(NavigationMesh* navMesh)
{
	this->navMesh = navMesh;
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

void EnemyGameObject::MoveAlongPath() 
{
    Vector3 pos = this->transform.GetPosition();

    if (outPathIndex < 0)
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

    this->transform.SetPosition(pos += (dir * 0.5f));
   // this->GetPhysicsObject()->AddForce(dir * 5.0f);
}

void EnemyGameObject::Update(float dt) 
{

    if (state != Ongoing) {
        sequence->Reset();
        state = Ongoing;
    }

    state = sequence->Execute(dt);
    DisplayPathfinding();
    MoveAlongPath();
}

