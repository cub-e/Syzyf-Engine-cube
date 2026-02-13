#include <BoundingBox.h>

#include <glm/gtc/matrix_access.hpp>

BoundingBox::BoundingBox(const glm::vec3& minCorner, const glm::vec3& maxCorner) {
	this->center = (minCorner + maxCorner) * 0.5f;

	const glm::vec3 extents = glm::abs((minCorner - maxCorner) * 0.5f);

	this->axisU = glm::vec4(1, 0, 0, extents.x);
	this->axisV = glm::vec4(0, 1, 0, extents.y);
	this->axisW = glm::vec4(0, 0, 1, extents.z);
}
BoundingBox::BoundingBox(const glm::vec3& center, const glm::vec3& extents, const glm::vec3& axisU, const glm::vec3& axisV, const glm::vec3& axisW):
center(center),
axisU(glm::vec4(axisU, extents.x)),
axisV(glm::vec4(axisV, extents.y)),
axisW(glm::vec4(axisW, extents.z)) { }

BoundingBox BoundingBox::CenterAndExtents(const glm::vec3& center, const glm::vec3& extents) {
	return BoundingBox(
		center,
		extents,
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1)
	);
}
BoundingBox BoundingBox::CenterAndExtents(const glm::vec3& center, const glm::vec3& extents, const glm::quat& rotation) {
	return BoundingBox(
		center,
		extents,
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1)
	);
}

glm::vec3 BoundingBox::GetExtents() const {
	return glm::vec3(this->axisU.w, this->axisV.w, this->axisW.w);
}
glm::vec3 BoundingBox::GetCenter() const {
	return this->center;
}

BoundingBox BoundingBox::Transform(const glm::mat4& transformation) const {
	BoundingBox result = *this;

	result.center = transformation * glm::vec4(result.center, 1);
	
	glm::vec3 scale(
		glm::length(glm::column(transformation, 0)),
		glm::length(glm::column(transformation, 1)),
		glm::length(glm::column(transformation, 2))
	);

	result.axisU = glm::vec4(
		glm::vec3(transformation * glm::vec4(glm::vec3(result.axisU), 0.0)),
		result.axisU.w
	);
	result.axisV = glm::vec4(
		glm::vec3(transformation * glm::vec4(glm::vec3(result.axisV), 0.0)),
		result.axisV.w
	);
	result.axisW = glm::vec4(
		glm::vec3(transformation * glm::vec4(glm::vec3(result.axisW), 0.0)),
		result.axisW.w
	);

	return result;
}
