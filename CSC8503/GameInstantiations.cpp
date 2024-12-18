#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "EnemyGameObject.h"

#include "Kitten.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

using namespace NCL;
using namespace CSC8503;


VictoryPlatform* TutorialGame::AddVictoryPlatformToWorld(const Vector3& position, Vector3 dimensions) {


	VictoryPlatform* victoryPlatform = new VictoryPlatform([&](bool hasWon) {EndGame(hasWon);});
	OBBVolume* volume = new OBBVolume(dimensions);

	victoryPlatform->SetBoundingVolume((CollisionVolume*)volume);
	victoryPlatform->GetTransform().SetScale(dimensions * 2.0f).SetPosition(position);
	victoryPlatform->SetRenderObject(new RenderObject(&victoryPlatform->GetTransform(), cubeMesh, basicTex, basicShader));
	victoryPlatform->SetPhysicsObject(new PhysicsObject(&victoryPlatform->GetTransform(), victoryPlatform->GetBoundingVolume()));
	victoryPlatform->GetPhysicsObject()->SetInverseMass(0);
	victoryPlatform->GetPhysicsObject()->InitCubeInertia();
	victoryPlatform->GetRenderObject()->SetColour(Vector4(0, 1.0f, 0, 1.0f));

	world->AddGameObject(victoryPlatform);
	updateObjects.push_back(victoryPlatform);

	return victoryPlatform;
}


GameObject* TutorialGame::AddNavMeshToWorld(const Vector3& position, Vector3 dimensions)
{
	navMesh = new NavigationMesh("smalltest.navmesh");
	GameObject* navMeshObject = new GameObject();
	for (size_t i = 0; i < navigationMesh->GetSubMeshCount(); ++i)
	{
		if (navigationMesh->GetSubMesh(i)->count != 36)
			continue;

		std::vector<Vector3> vertices = GetVertices(navigationMesh, i);

		Vector3 dimensions, localPosition;
		Quaternion rotationMatrix;

		CalculateCubeTransformations(vertices, localPosition, dimensions, rotationMatrix);

		GameObject* colliderObject = new GameObject();
		OBBVolume* volume = new OBBVolume(dimensions);

		colliderObject->SetBoundingVolume((CollisionVolume*)volume);
		colliderObject->GetTransform().SetScale(dimensions * 2.0f).SetPosition(localPosition).SetOrientation(rotationMatrix);
		colliderObject->SetRenderObject(new RenderObject(&colliderObject->GetTransform(), cubeMesh, basicTex, basicShader));

		colliderObject->SetPhysicsObject(new PhysicsObject(&colliderObject->GetTransform(), colliderObject->GetBoundingVolume()));
		colliderObject->GetPhysicsObject()->SetInverseMass(0);
		colliderObject->GetPhysicsObject()->InitCubeInertia();
		colliderObject->SetLayerID(Layers::LayerID::Default);

		world->AddGameObject(colliderObject);
	}
	return navMeshObject;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	players = new PlayerGameObject();
	SphereVolume* volume = new SphereVolume(1.0f);

	players->SetBoundingVolume((CollisionVolume*)volume);
	players->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	players->SetLayerID(Layers::LayerID::Player);
	players->SetRenderObject(new RenderObject(&players->GetTransform(), catMesh, nullptr, basicShader));
	players->SetPhysicsObject(new PhysicsObject(&players->GetTransform(), players->GetBoundingVolume()));

	players->GetPhysicsObject()->SetInverseMass(inverseMass);
	players->GetPhysicsObject()->InitSphereInertia();
	players->AddToIgnoredLayers(Layers::Enemy);
	players->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1.0f));
	players->SetController(controller);


	world->AddGameObject(players);
	updateObjects.push_back(players);

	return players;
}


GameObject* TutorialGame::AddSphereCastToWorld()
{
	float radius = 5;
	GameObject* sphere = new GameObject();
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);

	sphere->SetLayerID(Layers::Ignore_Collisions);
	sphere->SetTag(Tags::CursorCast);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize).SetPosition(Vector3(0,0,0));

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(10.0f);
	sphere->GetPhysicsObject()->InitSphereInertia();

	sphere->GetRenderObject()->SetColour(Vector4(0, 1, 0, 0.8f));


	world->AddGameObject(sphere);
	sphereCast = sphere;
	return sphere;
}


Kitten* TutorialGame::AddKittenToWorld(const Vector3& position, float radius, GameObject* swarm, float inverseMass)
{
	Kitten* sphere = new Kitten(navMesh, swarm);
	Vector3 sphereSize = Vector3(radius, radius, radius);
	
	SphereVolume* volume = new SphereVolume(radius * 0.25f);

	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize).SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), kittenMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->SetTag(Tags::Kitten);

	kittens.push_back(sphere);
	world->AddGameObject(sphere);
	updateObjects.push_back(sphere);

	return sphere;
}

EnemyGameObject* TutorialGame::AddEnemyToWorld(const Vector3& position)
{
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	enemies = new EnemyGameObject(navMesh);
	enemies->SetRay([&](Ray& r, RayCollision& closestCollision, bool closestObject) -> bool { return world->Raycast(r, closestCollision, closestObject); });
	enemies->SetGetPlayer([&]() -> Vector3 { return GetPlayerPos(); });

	SphereVolume* volume = new SphereVolume(0.6f);
	//OBBVolume* volume = new OBBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	enemies->SetBoundingVolume((CollisionVolume*)volume);
	enemies->SetLayerID(Layers::LayerID::Enemy);

	enemies->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	
	enemies->SetRenderObject(new RenderObject(&enemies->GetTransform(), enemyMesh, nullptr, basicShader));
	enemies->SetPhysicsObject(new PhysicsObject(&enemies->GetTransform(), enemies->GetBoundingVolume()));

	enemies->GetPhysicsObject()->SetInverseMass(inverseMass);
	enemies->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(enemies);

	updateObjects.push_back(enemies);
	return enemies;
}


Swarm* TutorialGame::AddSwarmToWorld(const Vector3& position)
{
	float meshSize = 0.6f;
	float inverseMass = 0.5f;

	swarm = new Swarm(navMesh);
	SphereVolume* volume = new SphereVolume(meshSize);

	swarm->SetGetPlayer([&]() -> Vector3 { return GetPlayerPos(); });
	swarm->SetLayerID(Layers::LayerID::Enemy);
	swarm->SetBoundingVolume((CollisionVolume*)volume);

	swarm->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);
	swarm->SetRenderObject(new RenderObject(&swarm->GetTransform(), sphereMesh, nullptr, basicShader));

	swarm->SetPhysicsObject(new PhysicsObject(&swarm->GetTransform(), swarm->GetBoundingVolume()));

	swarm->GetPhysicsObject()->SetInverseMass(inverseMass);
	swarm->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(swarm);


	auto offset = Vector3(0.3f, 0, 0.3f);

	for (float i = 0; i < 93; i++) {
		swarm->AddObjectToSwarm(AddKittenToWorld(position + (offset * i), 1, swarm));
	}

	updateObjects.push_back(swarm);
	return swarm;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position)
{
	GameObject* floor = new GameObject();
	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);

	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetScale(floorSize * 2.0f).SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	return floor;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass)
{
	GameObject* sphere = new GameObject();
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);

	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetScale(sphereSize).SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);
	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();
	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	return cube;
}