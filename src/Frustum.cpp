#include <Frustum.h>

Plane::Plane(const glm::vec3& normal, float distance):
normal(normal),
distance(distance) { }

Frustum::Frustum(const Plane& top,
        const Plane& bottom,
        const Plane& left,
        const Plane& right,
        const Plane& nearPlane,
        const Plane& farPlane
):
top(top),
bottom(bottom),
left(left),
right(right),
nearPlane(nearPlane),
farPlane(farPlane) { }