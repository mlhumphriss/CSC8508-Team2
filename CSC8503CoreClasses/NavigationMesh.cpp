#include "NavigationMesh.h"
#include "Assets.h"
#include "Maths.h"
#include <fstream>
#include "Mesh.h"
#include "RenderObject.h"

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

	for (int i = 0; i < numVertices; ++i) 
    {
		Vector3 vert;
		file >> vert.x;
		file >> vert.y;
		file >> vert.z;

		allVerts.emplace_back(vert);
	}

	allTris.resize(numIndices / 3);

	for (int i = 0; i < allTris.size(); ++i) 
    {
		NavTri* tri = &allTris[i];
		file >> tri->indices[0];
		file >> tri->indices[1];
		file >> tri->indices[2];

		tri->centroid = allVerts[tri->indices[0]] + allVerts[tri->indices[1]] + allVerts[tri->indices[2]];
		tri->centroid = allTris[i].centroid / 3.0f;
		tri->triPlane = Plane::PlaneFromTri(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);
		tri->area = Maths::AreaofTri3D(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);
	}
	for (int i = 0; i < allTris.size(); ++i) 
    {
		NavTri* tri = &allTris[i];
		for (int j = 0; j < 3; ++j) 
        {
			int index = 0;
			file >> index;
			if (index != -1) 
				tri->neighbours[j] = &allTris[index];
		}
	}
}

NavigationMesh::~NavigationMesh()
{
}



bool NavigationMesh::NodeInList(AStarNode* n, std::vector<AStarNode*>& list) const 
{
    std::vector<AStarNode*>::iterator i = std::find(list.begin(), list.end(), n);
    return i == list.end() ? false : true;
}

auto NavigationMesh::RemoveBestNode(std::vector<AStarNode*>& list)
{
    auto bestIt = list.begin();
    for (auto it = list.begin(); it != list.end(); ++it) {
        if ((*it)->f < (*bestIt)->f)
            bestIt = it;
    }
    AStarNode* bestNode = *bestIt;
    list.erase(bestIt);
    return bestNode;
}

float NavigationMesh::Heuristic(const Vector3& a, const Vector3& b) {
    return Vector::Length(b - a);
};

auto NavigationMesh::FindNodeInList(const NavTri* tri, const std::vector<AStarNode*>& list) 
{
    for (auto node : list) {
        if (node->tri == tri) 
            return node;
    }
    return static_cast<AStarNode*>(nullptr);
}


bool NavigationMesh::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
    const NavTri* start = GetTriForPosition(from);
    const NavTri* end = GetTriForPosition(to);

    if (!start || !end)
        return false;

    std::vector<AStarNode*> openList;
    std::vector<AStarNode*> closedList;

    AStarNode* startNode = new AStarNode(start, nullptr, 0, Heuristic(from, to)); 
    openList.push_back(startNode);

    AStarNode* currentNode = nullptr;

    while (!openList.empty()) {
        currentNode = RemoveBestNode(openList);

        if (currentNode->tri == end) {
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

            float g = currentNode->g + Heuristic(currentNode->tri->centroid, neighborTri->centroid);
            float h = Heuristic(neighborTri->centroid, to); 

            AStarNode* inOpen = FindNodeInList(neighborTri, openList);
            AStarNode* inClosed = FindNodeInList(neighborTri, closedList);

            if (!inOpen && !inClosed) {
                openList.push_back(new AStarNode(neighborTri, currentNode, g, h));
            }
            else if (inOpen && g < inOpen->g) {
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



bool NavigationMesh::HasLineOfSight(const Vector3& start, const Vector3& end) const {
    Vector3 direction = Vector::Normalise(end - start);
    float distance = Vector::Length(end - start);
    float u, d;

    for (const NavTri& tri : allTris) {

        Vector3 v0 = allVerts[tri.indices[0]];
        Vector3 v1 = allVerts[tri.indices[1]];
        Vector3 v2 = allVerts[tri.indices[2]];

        if (Maths::RayIntersectsTriangle(start, direction, v0, v1, v2, distance, u, d))
            return false;
    }
    return true; 
}


void NavigationMesh::SmoothPath(NavigationPath& path) {
    if (path.IsEmpty())
        return;

    Vector3 currentPoint;
    if (!path.PopWaypoint(currentPoint))
        return;

    NavigationPath smoothedPath;
    smoothedPath.PushWaypoint(currentPoint);

    Vector3 nextPoint;
    while (path.PopWaypoint(nextPoint)) 
    {
        if (HasLineOfSight(currentPoint, nextPoint)) 
            currentPoint = nextPoint;
        else {
            smoothedPath.PushWaypoint(currentPoint);
            currentPoint = nextPoint;
        }
    }

    smoothedPath.PushWaypoint(currentPoint);
    path = smoothedPath;
}

const NavigationMesh::NavTri* NavigationMesh::GetTriForPosition(const Vector3& pos) const {
    const NavTri* closestTri = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    Vector3 rayOrigin = pos;
    Vector3 rayDirection(0, -1, 0);

    float lastDist = -std::numeric_limits<float>::max();  

    for (const NavTri& t : allTris) {
        const Vector3& v0 = allVerts[t.indices[0]];
        const Vector3& v1 = allVerts[t.indices[1]];
        const Vector3& v2 = allVerts[t.indices[2]];

        float tDist = 0.0f;
        float u = 0.0f;
        float v = 0.0f;

        bool intersects = Maths::RayIntersectsTriangle(rayOrigin, rayDirection, v0, v1, v2, tDist, u, v);

        if (intersects && tDist > 0.0f) {
            if (fabs(tDist - lastDist) > 0.001f) {  
                lastDist = tDist;    
                if (tDist < closestDistance) {
                    closestDistance = tDist;
                    closestTri = &t;
                }
            }
        }
    }
    return closestTri;
}
