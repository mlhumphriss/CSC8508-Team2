#pragma once
#include <vector>

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		class NavigationPath		{
		public:
			NavigationPath() {}
			~NavigationPath() {}

			void clear() {
				waypoints.clear();
			}
			void PushWaypoint(const Vector3& wp) {
				waypoints.emplace_back(wp);
			}
			bool PopWaypoint(Vector3& waypoint) 
			{
				if (waypoints.empty()) 
					return false;
				waypoint = waypoints.back();
				waypoints.pop_back();
				return true;
			}

			bool IsEmpty() {
				return waypoints.empty();
			}

		protected:

			std::vector <Vector3> waypoints;
		};
	}
}

