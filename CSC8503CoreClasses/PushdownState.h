#pragma once

namespace NCL {
	namespace CSC8503 {
		class PushdownState
		{
		public:



			enum PushdownResult {
				Push, Pop, NoChange
			};			
			
			typedef std::function<PushdownResult(float, PushdownState**)> OnUpdateFunction;
			typedef std::function<void()> OnAwakeFunction;

			PushdownState() {}

			PushdownState(OnAwakeFunction onAwake, OnUpdateFunction onUpdate)
			{
				this->onAwake = onAwake;
				this->onUpdate = onUpdate;
			}

			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) 
			{
				if (onUpdate != nullptr) 
					return onUpdate(dt, pushFunc);
			}

			virtual void OnAwake() 
			{
				if (onAwake != nullptr)
					onAwake();
			}

			virtual void OnSleep() {}
			
		protected:
			OnAwakeFunction onAwake;
			OnUpdateFunction onUpdate;
		};
	}
}