#include "../NCLCoreClasses/KeyboardMouseController.h"
#include "NavigationGrid.h"
#include "EnemyGameObject.h"
#include "NavigationMesh.h"
#include "MainMenu.h"
#include "Swarm.h"



#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void SetPause(bool state);
			void InitWorld();
			void BridgeConstraintTest();
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			bool SelectObject();
			void MoveSelectedObject();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddNavMeshToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddPlayerToWorld(const Vector3& position);
			EnemyGameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddKittenToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);

			Swarm* AddSwarmToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);


			Vector3 GetPlayerPos();
			void DisplayPathfinding();
			void TestPathfinding();

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			void  CalculateCubeTransformations(const std::vector<Vector3>& vertices, Vector3& position, Vector3& scale, Quaternion& rotation);
			std::vector<Vector3>  GetVertices(Mesh* navigationMesh, int i);


#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inPause = false;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			Mesh* navigationMesh = nullptr;
			NavigationPath outPath;
			NavigationMesh* navMesh = nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	catMesh		= nullptr;
			Mesh*	kittenMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			MainMenu* mainMenu = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}


			std::vector<Vector4> colors = {
				Vector4(1.0f, 0.0f, 0.0f, 1.0f), // Red
				Vector4(0.0f, 1.0f, 0.0f, 1.0f), // Green
				Vector4(0.0f, 0.0f, 1.0f, 1.0f), // Blue
				Vector4(1.0f, 1.0f, 0.0f, 1.0f), // Yellow
				Vector4(1.0f, 0.0f, 1.0f, 1.0f), // Magenta
				Vector4(0.0f, 1.0f, 1.0f, 1.0f)  // Cyan
			};

			GameObject* objClosest = nullptr;
			GameObject* players = nullptr;
			Swarm* swarm = nullptr;

			EnemyGameObject* enemies = nullptr;

		};
	}
}

