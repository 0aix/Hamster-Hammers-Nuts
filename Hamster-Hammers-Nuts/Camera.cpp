#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Hamster
{
	Camera::Camera()
	{
		aspect = 4.0f / 3.0f;
		near = 0.1f;
		far = 1000.0f;
		scale = 12.5f;
	}

	void Camera::set(float radius, float elevation, float azimuth, glm::vec3 const & target)
	{
		transform.position = radius * glm::vec3(std::cos(elevation) * std::cos(azimuth),
			std::cos(elevation) * std::sin(azimuth),
			std::sin(elevation)) +
			target;

		glm::vec3 out = -glm::normalize(target - transform.position);
		glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
		up = glm::normalize(up - glm::dot(up, out) * out);
		glm::vec3 right = glm::cross(up, out);

		transform.rotation = glm::quat_cast(glm::mat3(right, up, out));
		transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	glm::mat4 Camera::make_projection() const
	{
		return glm::ortho(scale * -aspect, scale * aspect, -scale, scale, near, far);
	}
}
