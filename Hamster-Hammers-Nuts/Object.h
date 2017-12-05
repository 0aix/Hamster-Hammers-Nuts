#pragma once

#include "Mesh.h"
#include "GL.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Hamster
{
	struct Transform
	{
		glm::mat4 make_local_to_world() const;
		glm::mat4 make_world_to_local() const;

		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	};

	struct Object
	{
		Transform transform;
		Mesh mesh;
		Animation anim;
		bool animated;

		glm::vec3 velocity;
		float av;
		glm::vec3 rotation_axis;
		float height;
		float width;
		float length;
		glm::quat base_rotation;
	};

	struct Camera
	{
		Camera();
		void set(float radius, float elevation, float azimuth, glm::vec3& target);
		glm::mat4 make_projection() const;

		Transform transform;
		float aspect;
		float near;
		float far;
		float scale;
	};

	struct Light
	{
		Transform transform;
		glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f);
	};
}