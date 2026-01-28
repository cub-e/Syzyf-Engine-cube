#pragma once

#include <glm/glm.hpp>

struct Plane {
	glm::vec3 normal;
	float distance;

	Plane() = default;
	Plane(const glm::vec3& normal, float distance);
};

struct Frustum {
	Plane top;
	Plane bottom;
	Plane left;
	Plane right;
	Plane nearPlane;
	Plane farPlane;

	Frustum() = default;
	Frustum(const Plane& top,
	        const Plane& bottom,
	        const Plane& left,
	        const Plane& right,
	        const Plane& nearPlane,
	        const Plane& farPlane
	);
};