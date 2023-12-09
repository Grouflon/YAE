#pragma once

#include <yae/types.h>

#include <core/containers/Pool.h>
#include <yae/SceneGraphNode.h>
#include <yae/ID.h>

namespace yae {

class Scene;

// Elements
class YAE_API Entity
{
public:
	void init(ID<Entity> _id, ID<Scene> _sceneId, const char* _name);
	void shutdown();

	const SceneGraphNode& transform() const;
	SceneGraphNode& transform();

//private:
	ID<Entity> m_id;
	ID<Scene> m_scene;
	ID<SceneGraphNode> m_transform;
	String64 m_name;
};

class YAE_API Scene
{
public:
	void init(ID<Scene> _id, const char* _name);
	void shutdown();

//private:
	ID<Scene> m_id;
	String64 m_name;
};

// System
class YAE_API SceneSystem
{
public:
	void init();
	void shutdown();

	ID<Entity> createEntity(const char* _name, ID<Scene> _sceneId = ID<Scene>());
	void destroyEntity(ID<Entity> _id);
	const Entity* getEntity(ID<Entity> _id) const;
	Entity* getEntity(ID<Entity> _id);

	ID<Scene> createScene(const char* _name);
	void destroyScene(ID<Scene> _id);
	const Scene* getScene(ID<Scene> _id) const;
	Scene* getScene(ID<Scene> _id);

	ID<SceneGraphNode> createSceneGraphNode();
	void destroySceneGraphNode(ID<SceneGraphNode> _id);
	const SceneGraphNode* getSceneGraphNode(ID<SceneGraphNode> _id) const;
	SceneGraphNode* getSceneGraphNode(ID<SceneGraphNode> _id);

//private:
	Pool<Entity> m_entityPool;
	Pool<Scene> m_scenePool;
	Pool<SceneGraphNode> m_sceneGraphNodePool;
};

YAE_API SceneSystem& sceneSystem();

} // namespace yae
