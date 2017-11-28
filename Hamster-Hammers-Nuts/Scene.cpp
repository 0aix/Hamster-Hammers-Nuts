#include "Scene.h"
#include "Graphics.h"

namespace Hamster
{
	Scene::Scene()
	{
		current_time = std::chrono::high_resolution_clock::now();
		previous_time = current_time;
		elapsed = 0.0f;
	}

	Object& Scene::AddObject(const std::string& name, unsigned int meshID, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
	{
		Object object;
		object.transform.position = position;
		object.transform.rotation = rotation;
		object.transform.scale = scale;
		object.mesh = Mesh(meshID);
		object.animated = false;
		objects.push_back(object);
		return objects.back();
	}
	
	//Object& Scene::AddObject(const std::string& name, unsigned int meshID, unsigned int sknID, unsigned int animID, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
	//{
	//	Object object;
	//	object.transform.position = position;
	//	object.transform.rotation = rotation;
	//	object.transform.scale = scale;
	//	object.mesh = Mesh(meshID, sknID, animID);
	//	objects[name] = object;
	//	return objects[name];
	//}
	//
	//void Scene::RotateObject(const std::string& name, float degrees, glm::vec3 axis)
	//{
	//	if (degrees == 0.0f)
	//		return;
	//	objects[name].transform.rotation = glm::rotate(objects[name].transform.rotation, degrees, axis);
	//}
}