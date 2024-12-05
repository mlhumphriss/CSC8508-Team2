/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Maths.h"
#include "Vector.h"

namespace NCL {
	namespace Maths {
		void ScreenBoxOfTri(const Vector3& v0, const Vector3& v1, const Vector3& v2, Vector2& topLeft, Vector2& bottomRight) {
			topLeft.x = std::min(v0.x, std::min(v1.x, v2.x));
			topLeft.y = std::min(v0.y, std::min(v1.y, v2.y));

			bottomRight.x = std::max(v0.x, std::max(v1.x, v2.x));
			bottomRight.y = std::max(v0.y, std::max(v1.y, v2.y));
		}

		int ScreenAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c) {
			int area =(int) (((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) -
				((b.x * a.y) + (c.x * b.y) + (a.x * c.y)));
			return (area >> 1);
		}

		float SignedAreaof2DTri(const Vector3 &a, const Vector3 &b, const Vector3 & c) {
			float area = ((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) -
				((b.x * a.y) + (c.x * b.y) + (a.x * c.y));
			return (area * 0.5f);
		}

		float AreaofTri3D(const Vector3 &a, const Vector3 &b, const Vector3 & c) {
			Vector3 area = Vector::Cross(a - b, a - c);
			return Vector::Length(area) * 0.5f;
		}

		bool Maths::RayIntersectsEdge(const Vector3& rayStart, const Vector3& rayDir, float& distance, const Vector3& edgeStart, const Vector3& edgeEnd)
		{
			Vector3 edge = edgeEnd - edgeStart;
			Vector3 edgeNormal = Vector3(-edge.z, 0, edge.x); 

			float dot = Vector::Dot(rayDir, edgeNormal);

			if (fabs(dot) < 1e-6f)
				return false;
			
			distance = Vector::Dot(edgeNormal, edgeStart - rayStart) / dot;

			if (distance < 0) 
				return false;

			Vector3 intersection = rayStart + rayDir * distance;
			Vector3 edgeToIntersection = intersection - edgeStart;

			float edgeLengthSq = Vector::LengthSquared(edge);
			float projection = Vector::Dot(edgeToIntersection, edge) / edgeLengthSq;

			return projection >= 0.0f && projection <= 1.0f;
		}


		float RandomValue(float min, float max) {
			float floatValue = rand() / (float)RAND_MAX;
			float range = max - min;
			return min + (range * floatValue);
		}
	}
}