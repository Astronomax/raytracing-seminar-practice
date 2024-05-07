#ifndef RAYTRACING_TRANSFORM_HPP
#define RAYTRACING_TRANSFORM_HPP

#include "glm/vec3.hpp"
#include "glm/geometric.hpp"
#include <glm/gtc/quaternion.hpp>

#include <cstddef>
#include <cstring>

class Transform {
public:

    Transform();

    explicit Transform(float matrix[4][4]);

    Transform(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale);

    Transform prod(const Transform &other) const;

    glm::vec3 transform(const glm::vec3 &p) const;

    float matrix_[4][4];
};

#endif //RAYTRACING_TRANSFORM_HPP
