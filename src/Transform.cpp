#include <Transform.hpp>

static Transform scale_to_transform(const glm::vec3 &scale) {
	float matrix[4][4] = {
		{scale.x, 0,       0,       0},
		{0,       scale.y, 0,       0},
		{0,       0,       scale.z, 0},
		{0,       0,       0,       1}
	};
	return Transform(matrix);
}

static Transform translation_to_transform(const glm::vec3 &translation) {
	float matrix[4][4] = {
		{1, 0, 0, translation.x},
		{0, 1, 0, translation.y},
		{0, 0, 1, translation.z},
		{0, 0, 0, 1            }
	};
	return Transform(matrix);
}

static Transform rotation_to_transform(const glm::quat &q) {
	float matrix[4][4] = {
		{2 * (q.w * q.w + q.x * q.x) - 1,	2 * (q.x * q.y - q.w * q.z),   	2 * (q.x * q.z + q.w * q.y),   	0},
		{2 * (q.x * q.y + q.w * q.z),   	2 * (q.w * q.w + q.y * q.y) - 1, 	2 * (q.y * q.z - q.w * q.x),   	0},
		{2 * (q.x * q.z - q.w * q.y),   	2 * (q.y * q.z + q.w * q.x),   	2 * (q.w * q.w + q.z * q.z) - 1,	0},
		{0, 				0, 					0, 					1}
	};
	return Transform(matrix);
}

Transform::Transform() {
	memset(matrix_, 0, sizeof(matrix_));
	matrix_[0][0] = matrix_[1][1] = matrix_[2][2] = matrix_[3][3] = 1;
}

Transform::Transform(float matrix[4][4]) {
	memcpy(matrix_, matrix, sizeof(matrix_));
}

Transform::Transform(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale) {
	*this = translation_to_transform(translation).prod(rotation_to_transform(rotation)).prod(
		scale_to_transform(scale));
}

Transform Transform::prod(const Transform &other) const {
	Transform prod;
	memset(prod.matrix_, 0, sizeof(prod.matrix_));
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				prod.matrix_[i][j] += matrix_[i][k] * other.matrix_[k][j];
	return prod;
}

glm::vec3 Transform::transform(const glm::vec3 &p) const {
	float result[3];
	for (int i = 0; i < 3; i++)
		result[i] = matrix_[i][0] * p.x + matrix_[i][1] * p.y + matrix_[i][2] * p.z + matrix_[i][3];
	return {result[0], result[1], result[2]};
}