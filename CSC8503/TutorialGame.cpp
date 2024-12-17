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

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) 
{

	world = new GameWorld();
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
	
	mainMenu = new MainMenu([&](bool state) -> void { this->SetPause(state);});	
	physics->UseGravity(true);
	world->UpdateWorld(0.1f);
	physics->Update(0.1f);
}

void TutorialGame::SetPause(bool state) {
	inPause = state;
}

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

	delete players;
	delete swarm;
}

Vector3 TutorialGame::GetPlayerPos() {
	return players->GetTransform().GetPosition();
}

void TutorialGame::UpdateCamera(float dt) {

	if (!inSelectionMode)
		world->GetMainCamera().UpdateCamera(dt);

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0, 1, 0));
		Matrix4 modelMat = Matrix::Inverse(temp);

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);
	}

}

void TutorialGame::UpdateObjectSelectMode(float dt) {

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);
		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);
		bool hit = world->Raycast(r, closestCollision, true, selectionObject, new std::vector<Layers::LayerID>({ Layers::LayerID::Player,  Layers::LayerID::Enemy }));

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
}

void TutorialGame::UpdateGame(float dt) 
{
	SphereCastWorld();

	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(true);

	mainMenu->Update(dt);
	renderer->Render();
	renderer->Update(dt);
	Debug::UpdateRenderables(dt);

	if (inPause)
		return;

	UpdateCamera(dt);
	UpdateObjectSelectMode(dt);

	for (auto& obj : updateObjects) {
		obj->Update(dt);
	}

	world->UpdateWorld(dt);
	physics->Update(dt);
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
	BridgeConstraintTest();
	InitMixedGridWorld(15, 15, 3.5f, 3.5f);


	InitGameExamples();
}

std::vector<Vector3> TutorialGame::GetVertices(Mesh* navigationMesh, int i)
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


void TutorialGame::SphereCastWorld()
{
	float t, u, v;
	Vector3 intersection = Vector3(0,25,0);
	Ray mouseToWorld = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

	Vector3 dir = Vector::Normalise(mouseToWorld.GetDirection());
	Vector3 pos = mouseToWorld.GetPosition();

	bool found = false;
	float closestT = std::numeric_limits<float>::max();
	Vector3 closestIntersection;

	for (size_t i = 0; i < navigationMesh->GetSubMeshCount(); ++i) {
		const SubMesh* subMesh = navigationMesh->GetSubMesh(i);

		for (size_t j = subMesh->start; j < subMesh->start + subMesh->count; ++j) {
			Vector3 a, b, c;
			if (!navigationMesh->GetTriangle(j, a, b, c))
				continue;

			float t, u, v;
			if (RayIntersectsTriangle(pos, dir, a, b, c, t, u, v) && t < closestT) {
				closestT = t;
				closestIntersection = pos + (dir * t);
				found = true;
			}
		}
	}

	if (found) 
		intersection = closestIntersection;

	sphereCast->GetTransform().SetPosition(intersection);

}

const bool DebugCubeTransforms = false;

void  TutorialGame::CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation)
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
	a = vertices[1] - vertices[2];
	b = vertices[4] - vertices[5];
	c = vertices[8] - vertices[9];

	if (DebugCubeTransforms) {
		Debug::DrawLine(vertices[1], vertices[2], Vector4(1, 0, 0, 1));
		Debug::DrawLine(vertices[4], vertices[5], Vector4(0, 0, 1, 1));
		Debug::DrawLine(vertices[8], vertices[9], Vector4(0, 1, 0, 1));
	}

	extent = Vector3(Vector::Length(a),Vector::Length(b),Vector::Length(c));

	Vector3 localX = Vector::Normalise(a); 
	Vector3 localY = Vector::Normalise(b); 
	Vector3 localZ = -Vector::Normalise(c); 

	Matrix3 rotationMatrix = Matrix3();

	rotationMatrix.SetColumn(2, Vector4(localZ, 0));
	rotationMatrix.SetColumn(1, Vector4(localY, 0));
	rotationMatrix.SetColumn(0, Vector4(-localX, 0));

	rotation = Quaternion(rotationMatrix);
	scale = extent * 0.5f;
}


void TutorialGame::InitGameExamples() 
{	
	AddSphereCastToWorld();
	AddNavMeshToWorld(Vector3(0, 0, 0), Vector3(1, 1, 1));
	AddPlayerToWorld(Vector3(-100, 22, 25));
	AddEnemyToWorld(Vector3(5, 30, 0)); 
	AddSwarmToWorld(Vector3(75, 22, -50));
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
			//if (rand() % 2) 
				//AddCubeToWorld(position, cubeDims);
			//else 
				AddSphereToWorld(position, sphereRadius);
		}
	}
}

void TutorialGame::BridgeConstraintTest() 
{
	Vector3 cubeSize = Vector3(3, 3, 3);
	float invCubeMass = 5;
	int numLinks = 12;
	float maxDistance = 10;
	float cubeDistance = 2;
	Vector3 startPos = Vector3(100, 90, 100);

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
	}
	if (inSelectionMode) {

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

	return false;
}

void TutorialGame::MoveSelectedObject() {
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



