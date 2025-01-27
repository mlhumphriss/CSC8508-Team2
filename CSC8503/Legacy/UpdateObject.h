#pragma once

namespace NCL {
    namespace CSC8503 {
        class UpdateObject {
        public:
            virtual void Update(float dt) = 0;
            virtual ~UpdateObject() = default;
        };
    }
}