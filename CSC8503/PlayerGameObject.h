
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "NavigationMesh.h"

#include "PhysicsObject.h"
#include "Ray.h"
#include "UpdateObject.h"
#include "Controller.h"

#include "NavMeshAgent.h"
#include "CollisionDetection.h"
#include "GameWorld.h"

namespace NCL {
    namespace CSC8503 {
        class PlayerGameObject : public UpdateObject, public GameObject {
        public:

            PlayerGameObject();
            ~PlayerGameObject();

            void Update(float dt) override
            {
   
            }

            virtual void OnCollisionBegin(GameObject* otherObject) override {
                std::cout << "OnCollisionBegin event occured!\n";
            }


        protected:
            Controller* playerController;


    }
}