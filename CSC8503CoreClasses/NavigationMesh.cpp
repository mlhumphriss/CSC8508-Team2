#include "NavigationMesh.h"
#include "Assets.h"
#include "Maths.h"
#include <fstream>
using namespace NCL;
using namespace CSC8503;
using namespace std;

NavigationMesh::NavigationMesh()
{
}

NavigationMesh::NavigationMesh(const std::string&filename)
{
	ifstream file(Assets::DATADIR + filename);

	int numVertices = 0;
	int numIndices	= 0;

	if (!file)
		std::cout << "No file found" << std::endl;


	file >> numVertices;
	file >> numIndices;

	for (int i = 0; i < numVertices; ++i) {
		Vector3 vert;
		file >> vert.x;
		file >> vert.y;
		file >> vert.z;

		allVerts.emplace_back(vert);
	}

	allTris.resize(numIndices / 3);

	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		file >> tri->indices[0];
		file >> tri->indices[1];
		file >> tri->indices[2];

		tri->centroid = allVerts[tri->indices[0]] +
			allVerts[tri->indices[1]] +
			allVerts[tri->indices[2]];

		tri->centroid = allTris[i].centroid / 3.0f;

		tri->triPlane = Plane::PlaneFromTri(allVerts[tri->indices[0]],
			allVerts[tri->indices[1]],
			allVerts[tri->indices[2]]);

		tri->area = Maths::AreaofTri3D(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);
	}
	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		for (int j = 0; j < 3; ++j) {
			int index = 0;
			file >> index;
			if (index != -1) {
				tri->neighbours[j] = &allTris[index];
			}
		}
	}
}

Vector3 NavigationMesh::GetNearestPoint(Vector3& point) {
	point = Vector3(allTris[10].indices[0], allTris[10].indices[1], allTris[10].indices[2]);
	return Vector3(allTris[0].indices[0], allTris[0].indices[1], allTris[0].indices[2]);
}


NavigationMesh::~NavigationMesh()
{
}

bool NavigationMesh::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
    const NavTri* start = GetTriForPosition(from);
    const NavTri* end = GetTriForPosition(to);

    if (!start || !end) {
        return false;
    }

    struct AStarNode {
        const NavTri* tri;
        AStarNode* parent;
        float g;
        float f; 

        AStarNode(const NavTri* tri, AStarNode* parent, float g, float h)
            : tri(tri), parent(parent), g(g), f(g + h) {}
    };

    auto heuristic = [](const Vector3& a, const Vector3& b) {
        return Vector::Length(b - a); 
        };

    auto findNodeInList = [](const NavTri* tri, const std::vector<AStarNode*>& list) {
        for (auto node : list) {
            if (node->tri == tri) {
                return node;
            }
        }
        return static_cast<AStarNode*>(nullptr);
    };

    auto removeBestNode = [](std::vector<AStarNode*>& list) {
        auto bestIt = list.begin();
        for (auto it = list.begin(); it != list.end(); ++it) {
            if ((*it)->f < (*bestIt)->f) {
                bestIt = it;
            }
        }
        AStarNode* bestNode = *bestIt;
        list.erase(bestIt);
        return bestNode;
     };

    std::vector<AStarNode*> openList;
    std::vector<AStarNode*> closedList;

    AStarNode* startNode = new AStarNode(start, nullptr, 0, heuristic(start->centroid, end->centroid));
    openList.push_back(startNode);

    while (!openList.empty()) {
        AStarNode* currentNode = removeBestNode(openList);

        if (currentNode->tri == end) 
        {
            AStarNode* pathNode = currentNode;
            while (pathNode != nullptr) {
                outPath.PushWaypoint(pathNode->tri->centroid);
                pathNode = pathNode->parent;
            }
            for (auto node : openList) delete node;
            for (auto node : closedList) delete node;
            return true;
        }

        for (int i = 0; i < 3; ++i) 
        { 
            const NavTri* neighborTri = currentNode->tri->neighbours[i];
            if (!neighborTri) continue;

            AStarNode* inClosed = findNodeInList(neighborTri, closedList);
            if (inClosed) continue;

            float g = currentNode->g + heuristic(currentNode->tri->centroid, neighborTri->centroid);
            float h = heuristic(neighborTri->centroid, end->centroid);

            AStarNode* inOpen = findNodeInList(neighborTri, openList);
            if (!inOpen) {
                openList.push_back(new AStarNode(neighborTri, currentNode, g, h));
            }
            else if (g < inOpen->g) {
                inOpen->g = g;
                inOpen->f = g + h;
                inOpen->parent = currentNode;
            }
        }
        closedList.push_back(currentNode);
    }

    for (auto node : openList) delete node;
    for (auto node : closedList) delete node;

    return false; 
}

/*
If you have triangles on top of triangles in a full 3D environment, you'll need to change this slightly,
as it is currently ignoring height. You might find tri/plane raycasting is handy.
*/

const NavigationMesh::NavTri* NavigationMesh::GetTriForPosition(const Vector3& pos) const {
	for (const NavTri& t : allTris) {
		Vector3 planePoint = t.triPlane.ProjectPointOntoPlane(pos);

		float ta = Maths::AreaofTri3D(allVerts[t.indices[0]], allVerts[t.indices[1]], planePoint);
		float tb = Maths::AreaofTri3D(allVerts[t.indices[1]], allVerts[t.indices[2]], planePoint);
		float tc = Maths::AreaofTri3D(allVerts[t.indices[2]], allVerts[t.indices[0]], planePoint);

		float areaSum = ta + tb + tc;

		if (abs(areaSum - t.area)  > 0.001f) { //floating points are annoying! Are we more or less inside the triangle?
			continue;
		}
		return &t;
	}
	return nullptr;
}