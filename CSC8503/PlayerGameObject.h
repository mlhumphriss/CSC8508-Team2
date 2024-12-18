#pragma once
#include "PhysicsObject.h"
#include "Ray.h"
#include "UpdateObject.h"
//#include "Controller.h"
#include "CollisionDetection.h"

namespace NCL {
    namespace CSC8503 {
        class PlayerGameObject : public UpdateObject, public GameObject {
        public:

            PlayerGameObject();
            ~PlayerGameObject();

            void SetController(const Controller& c) {
                activeController = &c;
            }

            void Update(float dt) override
            {
                if (activeController == nullptr)
                    return;

                Vector3 dir;
                yaw -= activeController->GetNamedAxis("XLook");

                if (yaw < 0) 
                    yaw += 360.0f;
                if (yaw > 360.0f) 
                    yaw -= 360.0f;

                float frameSpeed = speed *dt;
                Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

                dir += yawRotation * Vector3(0, 0, -activeController->GetNamedAxis("Forward")) * frameSpeed;
                dir += yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0) * frameSpeed;
                dir = Vector::Normalise(dir);

                Matrix3 offsetRotation = Matrix::RotationMatrix3x3(-45.0f, Vector3(0, 1, 0));
                dir = offsetRotation * dir;

                this->GetPhysicsObject()->AddForce(dir * speed);
                this->GetPhysicsObject()->RotateTowardsVelocity();
            }

            virtual void OnCollisionBegin(GameObject* otherObject) override {
                
            }


        protected:
            const Controller* activeController = nullptr;
            float speed = 10.0f;
            float	yaw;
        };
    }
}