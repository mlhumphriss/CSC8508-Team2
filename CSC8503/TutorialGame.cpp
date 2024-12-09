#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "EnemyGameObject.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	navigationMesh = renderer->LoadMesh("NavMeshObject.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	catMesh		= renderer->LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh	= renderer->LoadMesh("Kitten.msh");

	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	
{
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete navigationMesh;
	delete navMesh;
}

vector<Vector3> testNodes;
NavigationPath outPath;

float delta = 0.25f;
Vector3 startPos = Vector3(0, 0, 0);
Vector3 endPos = Vector3(25, 0, 5);
GameObject* apple;




void TutorialGame::TestPathfinding()
{
	testNodes.clear();

	Vector3 intersectionPoint = endPos;
	float t, u, v;

	Vector3 playerPos = endPos;
	Vector3 rayDir = Vector3(0, -1, 0);

	auto submeshCount = navigationMesh->GetSubMeshCount();
	auto x = navigationMesh->GetPositionData();
	auto triCount = navigationMesh->GetIndexCount() / 3;


	for (int i = 0; i < triCount; ++i) {

		Vector3 a, b, c;
		navigationMesh->GetTriangle(i, a, b, c);

		Debug::DrawLine(a,b);
		Debug::DrawLine(b,c);
		Debug::DrawLine(c,a);

		if (RayIntersectsTriangle(playerPos, rayDir, a, b, c, t, u, v))
		{
				intersectionPoint = playerPos + rayDir * t;
				//break;
		}
	}
	
	endPos = intersectionPoint;
	apple->GetTransform().SetPosition(intersectionPoint);


	Vector3 pos;
	bool found = navMesh->FindPath(startPos, endPos, outPath);
	navMesh->SmoothPath(outPath);

	while (outPath.PopWaypoint(pos))
	{
		testNodes.push_back(pos);
	}

}

void TutorialGame::DisplayPathfinding() 
{
	Debug::DrawLine(startPos, endPos, Vector4(1, 0, 0, 1));

	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

void TutorialGame::UpdateGame(float dt) 
{
	if (!inSelectionMode) 
		world->GetMainCamera().UpdateCamera(dt);
	if (testStateObject) 
		testStateObject -> Update(dt);

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0,1,0));
		Matrix4 modelMat = Matrix::Inverse(temp);

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);
	}

	TestPathfinding();
	DisplayPathfinding();
	UpdateKeys();

	if (useGravity) 
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	else 
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);
		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);
		bool hit = world->Raycast(r, closestCollision, true, selectionObject);

		if (hit) 
		{
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;
			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}		
	}

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() 
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; 
		physics->UseGravity(useGravity);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::T))
		endPos.x += delta;
	if (Window::GetKeyboard()->KeyDown(KeyCodes::F))
		endPos.z -= delta;
	if (Window::GetKeyboard()->KeyDown(KeyCodes::G))
		endPos.x -= delta;
	if (Window::GetKeyboard()->KeyDown(KeyCodes::H))
		endPos.z += delta;
}

void TutorialGame::LockedObjectMovement() 
{
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); 
	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);

	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) 
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) 
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) 
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() 
{
	world->ClearAndErase();
	physics->Clear();
	//BridgeConstraintTest();
	InitMixedGridWorld(15, 15, 3.5f, 3.5f);
	InitGameExamples();
	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));

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
	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	return cube;
}

std::vector<Vector3> GetVertices(Mesh* navigationMesh, int i)
{
	const SubMesh* subMesh = navigationMesh->GetSubMesh(i);
	const std::vector<unsigned int>& indices = navigationMesh->GetIndexData();
	const std::vector<Vector3>& positionData = navigationMesh->GetPositionData();
	std::vector<Vector3> vertices;

	for (size_t j = subMesh->start; j < subMesh->start + subMesh->count; j += 3) {
		unsigned int idx0 = indices[j];
		unsigned int idx1 = indices[j + 1];
		unsigned int idx2 = indices[j + 2];

		vertices.push_back(positionData[idx0]);
		vertices.push_back(positionData[idx1]);
		vertices.push_back(positionData[idx2]);
	}
	return vertices;
}


void CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation)
{
	Vector3 minBound(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector3 maxBound(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

	for (const auto& vertex : vertices) {
		minBound = Vector::Min(minBound, vertex);
		maxBound = Vector::Max(maxBound, vertex);
	}

	position = (minBound + maxBound) * 0.5f;
	Vector3 extent = maxBound - minBound;

	Vector3 a, b, c;
	a = vertices[1] - vertices[0];
	b = vertices[4] - vertices[5];
	c = vertices[8] - vertices[9];

	extent = Vector3(Vector::Length(a),Vector::Length(b),Vector::Length(c));

	Vector3 localX = Vector::Normalise(a); 
	Vector3 localY = Vector::Normalise(b); 
	Vector3 localZ = Vector::Normalise(c); 

	Matrix3 rotationMatrix = Matrix3();
	rotationMatrix.SetColumn(0, Vector4(localX, 0));
	rotationMatrix.SetColumn(1, Vector4(localY, 0));
	rotationMatrix.SetColumn(2, Vector4(localZ, 0));


	std::cout << "Checking" << std::endl;
	std::cout << localX.x<< ", " << localX.y  << ", " << localX.z << std::endl;	
	std::cout << localY.x << ", " << localY.y << ", " << localY.z << std::endl;
	std::cout << localZ.x << ", " << localZ.y << ", " << localZ.z << std::endl;


	rotation = Quaternion(rotationMatrix);
	scale = extent * 0.5f;
}


GameObject* TutorialGame::AddNavMeshToWorld(const Vector3& position, Vector3 dimensions) 
{
	navMesh = new NavigationMesh("smalltest.navmesh");
	GameObject* navMeshObject = new GameObject();
	//navMeshObject->SetRenderObject(new RenderObject(&navMeshObject->GetTransform(), navigationMesh, basicTex, basicShader));

	for (size_t i = 0; i < navigationMesh->GetSubMeshCount(); ++i)
	{
		if (navigationMesh->GetSubMesh(i)->count != 36)
			continue;  

		std::vector<Vector3> vertices = GetVertices(navigationMesh, i);

		Vector3 dimensions, localPosition;
		Quaternion rotationMatrix;

		CalculateCubeTransformations(vertices, localPosition, dimensions, rotationMatrix);

		GameObject* colliderObject = new GameObject();
		AABBVolume* volume = new AABBVolume(dimensions);

		colliderObject->SetBoundingVolume((CollisionVolume*)volume);
		colliderObject->GetTransform().SetScale(dimensions * 2.0f).SetPosition(localPosition).SetOrientation(rotationMatrix);
		colliderObject->SetRenderObject(new RenderObject(&colliderObject->GetTransform(), cubeMesh, basicTex, basicShader));

		colliderObject->SetPhysicsObject(new PhysicsObject(&colliderObject->GetTransform(), colliderObject->GetBoundingVolume()));
		colliderObject->GetPhysicsObject()->SetInverseMass(0);
		colliderObject->GetPhysicsObject()->InitCubeInertia();

		world->AddGameObject(colliderObject);
		std::cout << "Adding cube collider for submesh " << i << std::endl;
	}

	world->AddGameObject(navMeshObject);
	return navMeshObject;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();
	SphereVolume* volume  = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);
	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

EnemyGameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	EnemyGameObject* character = new EnemyGameObject();
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);
	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	return character;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();
	SphereVolume* volume = new SphereVolume(0.5f);

	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetScale(Vector3(2, 2, 2)).SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);
	return apple;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	apple = new GameObject();

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

void TutorialGame::InitGameExamples() 
{
	AddNavMeshToWorld(Vector3(0, 0, 0), Vector3(1, 1, 1));
	//AddPlayerToWorld(Vector3(0, 5, 0));
	//AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 100.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 100.0f, z * rowSpacing);
			if (rand() % 2) 
				AddCubeToWorld(position, cubeDims);
			else 
				AddSphereToWorld(position, sphereRadius);
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims)
{
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::BridgeConstraintTest() 
{
	Vector3 cubeSize = Vector3(8, 8, 8);
	float invCubeMass = 5;
	int numLinks = 10;
	float maxDistance = 30;
	float cubeDistance = 20;
	Vector3 startPos = Vector3(0, 0, 0);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}


bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) 
			{
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else 
				return false;
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) 
					lockedObject = nullptr;
				else 
					lockedObject = selectionObject;
			}
		}
	}
	else 
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	return false;
}

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) 
		return;

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) 
	{
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
		RayCollision closestCollision;

		if (world->Raycast(ray, closestCollision, true)) 
			if (closestCollision.node == selectionObject) 
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
	}
}



