#include "Scene.h"
#include "Graphics.h"
#include "Game.h"


namespace Hamster
{
	Scene::Scene()
	{
		current_time = std::chrono::high_resolution_clock::now();
		previous_time = current_time;
		elapsed = 0.0f;
	}

	Object& Scene::AddObject(const std::string& name, unsigned int meshID, glm::vec3& position, glm::vec3& dimension, glm::quat& rotation, glm::vec3& scale)
	{
		Object object;
		object.transform.position = position;
		object.transform.rotation = rotation;
		object.transform.scale = scale;
		object.height = dimension.z;
		object.length = dimension.x;
		object.width = dimension.y;
		object.mesh = Mesh(meshID);
		object.animated = false;
		objects.push_back(object);
		return objects.back();
	}
	
	void Scene::RotateObject(Object* obj, float degrees, glm::vec3 axis)
	{
		if (degrees == 0.0f) {
			return;
		}
		auto quart = obj->transform.rotation;
		quart = glm::rotate(quart, degrees, axis);
		obj->transform.rotation = quart;
	}

	void Scene::RotateDirection(Object* obj, Direction direction)
	{
		auto quart = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float degrees = 0.0f;
		if (direction == Direction::Up) {
			degrees = 0.5f*M_PI;
		}
		else if (direction == Direction::Down) {
			degrees = 1.5f*M_PI;
		}
		else if (direction == Direction::Left) {
			degrees = M_PI;
		}
		else if (direction == Direction::Right){
			degrees = 0.0f;
		}
		else if (direction == Direction::LeftUp) {
			degrees = 0.75f*M_PI;
		}
		else if (direction == Direction::RightUp) {
			degrees = 0.25f*M_PI;
		}
		else if (direction == Direction::LeftDown) {
			degrees = 1.25f*M_PI;
		}
		else if (direction == Direction::RightDown) {
			degrees = 1.75f*M_PI;
		}

		quart = glm::rotate(quart, degrees, glm::vec3(0.0f,0.0f,1.0f));
		obj->transform.rotation = quart;
	}
}