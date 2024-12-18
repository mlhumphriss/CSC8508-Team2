#pragma once
#include "NavigationMap.h"
#include "Plane.h"
#include <string>
#include "Mesh.h"
#include "RenderObject.h"
#include <vector>

namespace NCL {
	namespace CSC8503 {
		class NavigationMesh : public NavigationMap	{
		public:
			NavigationMesh();
			NavigationMesh(const std::string&filename);
			~NavigationMesh();


			void SmoothPath(NavigationPath& originalPath);
			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
		protected:			
			
			struct NavTri {
				Plane   triPlane;
				Vector3 centroid;
				float	area;
				NavTri* neighbours[3];

				int indices[3];

				NavTri() {
					area = 0.0f;
					neighbours[0] = nullptr;
					neighbours[1] = nullptr;
					neighbours[2] = nullptr;

					indices[0] = -1;
					indices[1] = -1;
					indices[2] = -1;
				}
			};

			struct AStarNode {
				const NavTri* tri;
				AStarNode* parent;
				float g;
				float f;

				AStarNode(const NavTri* tri, AStarNode* parent, float g, float h)
					: tri(tri), parent(parent), g(g), f(g + h) {}
			};

			const NavTri* GetTriForPosition(const Vector3& pos) const;
			bool NodeInList(AStarNode* n, std::vector<AStarNode*>& list) const;
			bool HasLineOfSight(const Vector3& start, const Vector3& end) const;
			auto RemoveBestNode(std::vector<AStarNode*>& list);
			auto FindNodeInList(const NavTri* tri, const std::vector<AStarNode*>& list);
			float Heuristic(const Vector3& a, const Vector3& b);


			std::vector<NavTri>		allTris;
			std::vector<Vector3>	allVerts;
		};
	}
}