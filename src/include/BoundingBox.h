#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct BoundingBox {
	glm::vec3 center;
	glm::vec4 axisU;
	glm::vec4 axisV;
	glm::vec4 axisW;

	BoundingBox() = default;

	BoundingBox(const glm::vec3& minCorner, const glm::vec3& maxCorner);
	BoundingBox(const glm::vec3& center, const glm::vec3& extents, const glm::vec3& axisU, const glm::vec3& axisV, const glm::vec3& axisW);

	static BoundingBox CenterAndExtents(const glm::vec3& center, const glm::vec3& extents);
	static BoundingBox CenterAndExtents(const glm::vec3& center, const glm::vec3& extents, const glm::quat& rotation);

	glm::vec3 GetExtents() const;
	glm::vec3 GetCenter() const;

	BoundingBox Transform(const glm::mat4& transformation) const;
};