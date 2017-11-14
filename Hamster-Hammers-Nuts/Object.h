#pragma once

#include "Mesh.h"
#include "GL.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Hamster
{
	struct Transform
	{
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

		glm::mat4 make_local_to_world() const;
		glm::mat4 make_world_to_local() const;
	};

	struct Camera
	{
		Transform transform;

		// camera parameters (orthographic)
		float fovy = glm::radians(60.0f); // vertical fov (in radians)
		float aspect = 4.0f / 3.0f; // x / y
		float near = 0.1f;
		float far = 1000.0f;
		float scale = 22.5f;

		void set(float radius, float elevation, float azimuth, glm::vec3& target);
		glm::mat4 make_projection() const;
	};

	struct Object
	{
		Transform transform;
		Mesh mesh;
	};

	struct Light
	{
		Transform transform;
		// light parameters (directional)
		glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f); // effectively, color
	};
}