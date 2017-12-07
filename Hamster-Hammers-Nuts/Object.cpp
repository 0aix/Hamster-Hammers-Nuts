#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Hamster
{
	glm::mat4 Transform::make_local_to_world() const
	{
		glm::mat4 translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
										glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
										glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
										glm::vec4(position, 1.0f));
		glm::mat4 rotate = glm::mat4_cast(rotation);
		glm::mat4 scale_mat = glm::mat4(glm::vec4(scale.x, 0.0f, 0.0f, 0.0f),
										glm::vec4(0.0f, scale.y, 0.0f, 0.0f),
										glm::vec4(0.0f, 0.0f, scale.z, 0.0f),
										glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		return translate * rotate * scale_mat;
	}

	glm::mat4 Transform::make_world_to_local() const
	{
		glm::vec3 inv_scale;
		inv_scale.x = (scale.x == 0.0f ? 0.0f : 1.0f / scale.x);
		inv_scale.y = (scale.y == 0.0f ? 0.0f : 1.0f / scale.y);
		inv_scale.z = (scale.z == 0.0f ? 0.0f : 1.0f / scale.z);
		glm::mat4 unscale = glm::mat4(glm::vec4(inv_scale.x, 0.0f, 0.0f, 0.0f),
									  glm::vec4(0.0f, inv_scale.y, 0.0f, 0.0f),
									  glm::vec4(0.0f, 0.0f, inv_scale.z, 0.0f),
									  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glm::mat4 unrotate = glm::mat4_cast(glm::inverse(rotation));
		glm::mat4 untranslate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
										  glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
										  glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
										  glm::vec4(-position, 1.0f));
		return unscale * unrotate * untranslate;
	}
}