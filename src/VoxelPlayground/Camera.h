#pragma once
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../Utils/ButtonMap.h"

class Camera
{
private:
	const float kSensitivity = 1.f;
	float kTargetDistance = 5.f;

public:
	glm::vec3 position; // The position of camera in free r
	glm::vec3 target, dir, up_vector; // what the camera looks at, the direction, up-vector
	float pitch, yaw, current_distance;
	Camera(glm::vec3 front);

	glm::mat4 getLookAt();
	glm::vec3 getPosition();
	void rotate(float pitchDiff, float yawDiff);
	void tick(float delta, ButtonMap bm);

	std::string toString();

private:
	void updateTargetPos(glm::vec3 new_target);
};
