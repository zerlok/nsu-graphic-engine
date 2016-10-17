#include "scene.hpp"

#include <algorithm>
#include <logger.hpp>


const Camera Scene::DEFAULT_CAMERA = Camera();
const std::string Scene::DEFAULT_NAME = "Scene";


Scene::Scene(const std::string& name, const Camera& camera)
	: Component(Component::Type::SCENE, name),
	  _meshes(),
	  _lights(),
	  _camera(camera),
	  _bgColor(Color::grey)
{
	logDebug << "Scene with camera "
			 << _camera.getName() << " created" << logEnd;
}


Scene::Scene(const Scene& scene)
	: Component(scene),
	  _meshes(scene._meshes),
	  _lights(scene._lights),
	  _camera(scene._camera),
	  _bgColor(scene._bgColor)
{
}


Scene::~Scene()
{
	logDebug << "Scene removed" << logEnd;
}


Scene& Scene::operator=(const Scene& scene)
{
	Component::operator=(scene);

	_meshes = scene._meshes;
	_lights = scene._lights;
	_camera = scene._camera;
	_bgColor = scene._bgColor;

	return (*this);
}


void Scene::addMesh(const Mesh& mesh)
{
	_meshes.push_back(mesh);
	logDebug << "Mesh: " << _meshes.back().getName()
			 << " added to scene " << _name << logEnd;
}


void Scene::addLight(const Light& lamp)
{
	_lights.push_back(lamp);
	logDebug << "Lamp: " << _lights.back().getName()
			 << " added to scene " << _name << logEnd;
}


const Meshes& Scene::getMeshes() const
{
	return _meshes;
}


const Lights& Scene::getLamps() const
{
	return _lights;
}


const Camera& Scene::getCamera() const
{
	return _camera;
}


void Scene::setCamera(const Camera& camera)
{
	logDebug << "Scene camera " << _camera.getName() << " (old)";
	_camera = camera;
	logDebug << " replaced with " << _camera.getName() << " (new)" << logEnd;
}


const Color&Scene::getBgColor() const
{
	return _bgColor;
}


void Scene::setBgColor(const Color& color)
{
	_bgColor = color;
}
