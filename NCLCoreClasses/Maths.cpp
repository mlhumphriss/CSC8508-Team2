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

		int ScreenAreaOfTri(const Vector3& a, const Vector3& b, const Vector3& c) {
			int area = (int)(((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) -
				((b.x * a.y) + (c.x * b.y) + (a.x * c.y)));
			return (area >> 1);
		}

		float SignedAreaof2DTri(const Vector3& a, const Vector3& b, const Vector3& c) {
			float area = ((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) -
				((b.x * a.y) + (c.x * b.y) + (a.x * c.y));
			return (area * 0.5f);
		}

		float AreaofTri3D(const Vector3& a, const Vector3& b, const Vector3& c) {
			Vector3 area = Vector::Cross(a - b, a - c);
			return Vector::Length(area) * 0.5f;
		}


		Vector3 Maths::ClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p) {
			Vector3 ab = b - a;
			float abLengthSq = Vector::Dot(ab, ab);

			if (abLengthSq == 0.0f)
				return a;
			float t = Vector::Dot(p - a, ab) / abLengthSq;
			t = std::clamp(t, 0.0f, 1.0f);
			return a + ab * t;
		}

		bool Maths::PointInTri3D(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c) {
			float areaABC = AreaofTri3D(a, b, c);
			float areaPBC = AreaofTri3D(point, b, c);
			float areaPCA = AreaofTri3D(point, c, a);
			float areaPAB = AreaofTri3D(point, a, b);

			return fabs(areaPBC + areaPCA + areaPAB - areaABC) < 0.001f;
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


		#define EPSILON 1e-5f

		bool Maths::RayIntersectsTriangle(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& v0, const Vector3& v1, const Vector3& v2, float distance)
		{
			Vector3 e1 = v1 - v0;
			Vector3 e2 = v2 - v0;
			Vector3 h = Vector::Cross(rayDir, e2);
			float a = Vector::Dot(e1, h);

			if (a > -EPSILON && a < EPSILON)
				return false;

			float f = 1.0f / a;
			Vector3 s = rayOrigin - v0;
			float u = f * Vector::Dot(s, h);

			if (u < 0.0f || u > 1.0f)
				return false;

			Vector3 q = Vector::Cross(s, e1);
			float v = f * Vector::Dot(rayDir, q);
			if (v < 0.0f || u + v > 1.0f)
				return false;

			float t = f * Vector::Dot(e2, q);
			return t > EPSILON && t < distance;
		}


		float RandomValue(float min, float max) {
			float floatValue = rand() / (float)RAND_MAX;
			float range = max - min;
			return min + (range * floatValue);
		}


		// Intersection distance & Barycentric coordinates
		bool RayIntersectsTriangle(const Vector3& orig, const Vector3& dir, const Vector3& v0, const Vector3& v1, const Vector3& v2, float& t, float& u, float& v)
		{
			Vector3 v0v1 = v1 - v0;
			Vector3 v0v2 = v2 - v0;
			Vector3 pvec = Vector::Cross(dir, v0v2);
			float det = Vector::Dot( v0v1, pvec);

			if (det < EPSILON) return false;
			if (fabs(det) < EPSILON) return false;

			float invDet = 1 / det;

			Vector3 tvec = orig - v0;
			u = Vector::Dot(tvec, pvec) * invDet;
			if (u < 0 || u > 1) return false;

			Vector3 qvec = Vector::Cross(tvec, v0v1);
			v =  Vector::Dot(dir, qvec) * invDet;
			if (v < 0 || u + v > 1) return false;

			t = Vector::Dot(v0v2, qvec) * invDet;
			return true;
		}
	}
}