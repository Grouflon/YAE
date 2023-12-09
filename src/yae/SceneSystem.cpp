#include "SceneSystem.h"

#include <yae/Application.h>

namespace yae {

void Entity::init(ID<Entity> _id, ID<Scene> _sceneId, const char* _name)
{
	m_id = _id;
	m_scene = _sceneId;
	m_name = _name;

	m_transform = sceneSystem().createSceneGraphNode();
}

void Entity::shutdown()
{
	m_transform->setParent(ID<SceneGraphNode>::INVALID);
	sceneSystem().destroySceneGraphNode(m_transform);
	m_transform = ID<SceneGraphNode>::INVALID;
}

const SceneGraphNode& Entity::transform() const
{
	return *m_transform;
}

SceneGraphNode& Entity::transform()
{
	return *m_transform;
}

void Scene::init(ID<Scene> _id, const char* _name)
{
	m_id = _id;
	m_name = _name;
}

void Scene::shutdown()
{
}

void SceneSystem::init()
{

}

void SceneSystem::shutdown()
{
	YAE_ASSERT(m_entityPool.size() == 0);
	YAE_ASSERT(m_scenePool.size() == 0);
}

ID<Entity> SceneSystem::createEntity(const char* _name, ID<Scene> _sceneId)
{
	ID<Entity> id = ID<Entity>(m_entityPool.add(Entity()), &m_entityPool);
	Entity* entity = id.get();
	YAE_ASSERT(entity != nullptr);

	entity->init(id, _sceneId, _name);

	return id; 
}

void SceneSystem::destroyEntity(ID<Entity> _id)
{
	YAE_ASSERT(_id.get() != nullptr);

	_id->shutdown();

	YAE_VERIFY(m_entityPool.remove(_id.id));
}

const Entity* SceneSystem::getEntity(ID<Entity> _id) const
{
	return m_entityPool.get(_id.id);
}

Entity* SceneSystem::getEntity(ID<Entity> _id)
{
	return m_entityPool.get(_id.id);
}

ID<Scene> SceneSystem::createScene(const char* _name)
{
	ID<Scene> id = ID<Scene>(m_scenePool.add(Scene()), &m_scenePool);
	Scene* scene = m_scenePool.get(id.id);
	YAE_ASSERT(scene != nullptr);

	scene->init(id, _name);

	return id; 
}

void SceneSystem::destroyScene(ID<Scene> _id)
{
	YAE_ASSERT(_id.get() != nullptr);

	_id->shutdown();

	YAE_VERIFY(m_scenePool.remove(_id.id));
}

const Scene* SceneSystem::getScene(ID<Scene> _id) const
{
	return m_scenePool.get(_id.id);
}

Scene* SceneSystem::getScene(ID<Scene> _id)
{
	return m_scenePool.get(_id.id);
}

ID<SceneGraphNode> SceneSystem::createSceneGraphNode()
{
	ID<SceneGraphNode> id = ID<SceneGraphNode>(m_sceneGraphNodePool.add(SceneGraphNode()), &m_sceneGraphNodePool);
	SceneGraphNode* sceneGraphNode = m_sceneGraphNodePool.get(id.id);
	YAE_ASSERT(sceneGraphNode != nullptr);

	sceneGraphNode->m_id = id;

	return id; 
}

void SceneSystem::destroySceneGraphNode(ID<SceneGraphNode> _id)
{
	YAE_VERIFY(m_sceneGraphNodePool.remove(_id.id));
}

const SceneGraphNode* SceneSystem::getSceneGraphNode(ID<SceneGraphNode> _id) const
{
	return m_sceneGraphNodePool.get(_id.id);
}

SceneGraphNode* SceneSystem::getSceneGraphNode(ID<SceneGraphNode> _id)
{
	return m_sceneGraphNodePool.get(_id.id);
}

SceneSystem& sceneSystem()
{
	return app().sceneSystem();
}

} // namespace yae
