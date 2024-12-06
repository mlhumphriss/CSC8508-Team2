/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Vector.h"

namespace NCL::Maths {
	//It's pi(ish)...
	static const float		PI = 3.14159265358979323846f;

	//It's pi...divided by 360.0f!
	static const float		PI_OVER_360 = PI / 360.0f;

	//Radians to degrees
	inline float RadiansToDegrees(float rads) {
		return rads * 180.0f / PI;
	};

	//Degrees to radians
	inline float DegreesToRadians(float degs) {
		return degs * PI / 180.0f;
	};


	Vector3 ClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p);
	float RandomValue(float min, float max);
	bool PointInTri3D(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c);
	void ScreenBoxOfTri(const Vector3& v0, const Vector3& v1, const Vector3& v2, Vector2& topLeft, Vector2& bottomRight);

	int ScreenAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c);
	float SignedAreaof2DTri(const Vector3 &a, const Vector3 &b, const Vector3 & c);



	bool RayIntersectsEdge(const Vector3& rayStart, const Vector3& rayDir, float& distance, const Vector3& edgeStart, const Vector3& edgeEnd);
	bool RayIntersectsTriangle(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& v0, const Vector3& v1, const Vector3& v2, float distance);
	bool RayIntersectsTriangle(const Vector3& orig, const Vector3& dir, const Vector3& v0, const Vector3& v1, const Vector3& v2, float& t, float& u, float& v);

	float AreaofTri3D(const Vector3 &a, const Vector3 &b, const Vector3 & c);
}