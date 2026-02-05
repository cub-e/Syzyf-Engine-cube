#pragma once

#include <concepts>
#include <vector>
#include <list>
#include <typeinfo>

#include <spdlog/spdlog.h>

#include <Transform.h>

class GameObject;
class SceneGraphics;
class SceneComponent;
class Light;

typedef void (GameObject::*MessageMethod)();

template<class T>
concept Updateable = requires (T a) {
	{ a.Update() } -> std::same_as<void>;
};

template<class T>
concept Awakeable = requires (T a) {
	{ a.Awake() } -> std::same_as<void>;
};

template<class T>
concept Renderable = requires (T a) {
	{ a.Render() } -> std::same_as<void>;
};

template<class T>
concept Enableable = requires (T a) {
	{ a.Enable() } -> std::same_as<void>;
};

template<class T>
concept Disableable = requires (T a) {
	{ a.Disable() } -> std::same_as<void>;
};

template<class T>
concept DrawsGizmos = requires (T a) {
	{ a.DrawGizmos() } -> std::same_as<void>;
};

template<class T>
concept DrawsImGui = requires (T a) {
	{ a.OnImGui() } -> std::same_as<void>;
};

class Scene;

class SceneNode {
	friend class Scene;
private:
	struct MessageReceiver {
		GameObject* objPtr;
		MessageMethod methodPtr;

		void Message();
	};

	SceneNode* parent;

	int id;
	std::string name;

	bool enabled;

	Scene* const scene;
	std::vector<GameObject*> objects;

	std::vector<MessageReceiver> updateable;
	std::vector<MessageReceiver> renderable;
	std::vector<MessageReceiver> drawGizmos;

	std::vector<SceneNode*> children;
	SceneTransform transform;

	SceneNode(Scene* scene);
	SceneNode() = delete;

	void RecalculateTransform();
public:
	~SceneNode();

	SceneTransform& GetTransform();
	SceneTransform::TransformAccess& LocalTransform();
	SceneTransform::TransformAccess& GlobalTransform();

	int GetID() const;

	std::string GetName() const;
	void SetName(const std::string& name);

	Scene* GetScene();

	bool IsEnabled() const;
	void SetEnabled(bool value);

	const std::vector<SceneNode*> GetChildren();
	SceneNode* GetParent();
	void SetParent(SceneNode* newParent);
	bool IsChildOf(const SceneNode* node);

	void MarkDirty();
	void MarkChildrenDirty();

	const std::vector<GameObject*> AttachedObjects();

	template<class T_GO, typename... T_Param>
		requires std::derived_from<T_GO, GameObject>
	T_GO* AddObject(T_Param... params);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	T_GO* GetObject() const;

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryGetObject(T_GO*& found) const;

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	std::vector<T_GO*> GetAllObjects() const;

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	T_GO* GetObjectInChildren() const;

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryGetObjectInChildren(T_GO*& found) const;

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	std::vector<T_GO*> GetAllObjectsInChildren() const;

	void DeleteObject(GameObject* obj);
};

class Scene {
	friend class SceneNode;
private:
	int nextSceneNodeID;
	int nextGameObjectID;

	std::vector<SceneComponent*> components;
	SceneNode* root;

	SceneGraphics* graphics;

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryCreateAwakeable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject> && Awakeable<T_GO>
	bool TryCreateAwakeable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryCreateUpdateable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject> && Updateable<T_GO>
	bool TryCreateUpdateable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryCreateRenderable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject> && Renderable<T_GO>
	bool TryCreateRenderable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryCreateEnableable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject> && Enableable<T_GO>
	bool TryCreateEnableable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryCreateDisableable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject> && Disableable<T_GO>
	bool TryCreateDisableable(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	bool TryCreateDrawingGizmos(SceneNode* node, T_GO* object);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject> && DrawsGizmos<T_GO>
	bool TryCreateDrawingGizmos(SceneNode* node, T_GO* object);

	void DeleteObjectInternal(GameObject* obj);
	void DeleteNodeInternal(SceneNode* node);
public:
	Scene();
	SceneNode* CreateNode();
	SceneNode* CreateNode(SceneNode* parent);
	SceneNode* CreateNode(const std::string& name);
	SceneNode* CreateNode(SceneNode* parent, const std::string& name);

	SceneGraphics* GetGraphics();

	SceneNode* GetRootNode();

	template<class T_GO, typename... T_Param>
		requires std::derived_from<T_GO, GameObject>
	T_GO* CreateObjectOn(SceneNode* node, T_Param... params);

	void DeleteObject(GameObject* obj);
	void DeleteNode(SceneNode* node);

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	std::vector<T_GO*> FindObjectsOfType();

	template<class T_SC>
		requires std::derived_from<T_SC, SceneComponent>
	T_SC* GetComponent();

	template<class T_SC>
		requires std::derived_from<T_SC, SceneComponent>
	bool TryGetComponent(T_SC*& component);

	template<class T_SC>
		requires std::derived_from<T_SC, SceneComponent>
	T_SC* GetOrCreateComponent();

	template<class T_SC>
		requires std::derived_from<T_SC, SceneComponent>
	T_SC* AddComponent();

	void Update();
	void Render();
	void DrawImGui();
};

#include <GameObject.h>
#include <GameObjectSystem.h>

template<class T_GO, typename... T_Param>
	requires std::derived_from<T_GO, GameObject>
T_GO* SceneNode::AddObject(T_Param... params) {
	return this->scene->CreateObjectOn<T_GO>(this, params...);
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
T_GO* SceneNode::GetObject() const {
	for (GameObject* obj : this->objects) {
		if (dynamic_cast<T_GO*>(obj)) {
			return (T_GO*) obj;
		}
	}

	return nullptr;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool SceneNode::TryGetObject(T_GO*& found) const {
	T_GO* ptr = GetObject<T_GO>();

	if (ptr) {
		found = ptr;
		return true;
	}

	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
std::vector<T_GO*> SceneNode::GetAllObjects() const {
	std::vector<T_GO*> result;

	for (GameObject* obj : this->objects) {
		T_GO* converted = dynamic_cast<T_GO*>(obj);

		if (converted) {
			result.push_back(converted);
		}
	}

	return result;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
T_GO* SceneNode::GetObjectInChildren() const {
	T_GO* result;

	if (TryGetObject(result)) {
		return result;
	}

	for (const auto& child : this->children) {
		if (child->TryGetObjectInChildren(result)) {
			return result;
		}
	}

	return nullptr;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool SceneNode::TryGetObjectInChildren(T_GO*& found) const {
	if (TryGetObject(found)) {
		return true;
	}

	for (const auto& child : this->children) {
		if (child->TryGetObjectInChildren(found)) {
			return true;
		}
	}

	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
std::vector<T_GO*> SceneNode::GetAllObjectsInChildren() const {
	std::vector<T_GO*> result = GetAllObjects<T_GO>();

	for (const auto& child : this->children) {
		std::vector<T_GO*> partial = child->GetAllObjectsInChildren<T_GO>();

		for (const auto& obj : partial) {
			result.push_back(obj);
		}
	}

	return result;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool Scene::TryCreateAwakeable(SceneNode* node, T_GO* object) {
	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject> && Awakeable<T_GO>
bool Scene::TryCreateAwakeable(SceneNode* node, T_GO* object) {
	object->Awake();

	return true;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool Scene::TryCreateUpdateable(SceneNode* node, T_GO* object) {
	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject> && Updateable<T_GO>
bool Scene::TryCreateUpdateable(SceneNode* node, T_GO* object) {
	node->updateable.push_back({ object, reinterpret_cast<MessageMethod>(&T_GO::Update) });

	return true;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool Scene::TryCreateEnableable(SceneNode* node, T_GO* object) {
	object->onEnable = nullptr;

	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject> && Enableable<T_GO>
bool Scene::TryCreateEnableable(SceneNode* node, T_GO* object) {
	object->onEnable = reinterpret_cast<MessageMethod>(&T_GO::Enable);

	object->Enable();

	return true;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool Scene::TryCreateDisableable(SceneNode* node, T_GO* object) {
	object->onDisable = nullptr;

	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject> && Disableable<T_GO>
bool Scene::TryCreateDisableable(SceneNode* node, T_GO* object) {
	object->onDisable = reinterpret_cast<MessageMethod>(&T_GO::Disable);

	return true;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool Scene::TryCreateRenderable(SceneNode* node, T_GO* object) {
	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject> && Renderable<T_GO>
bool Scene::TryCreateRenderable(SceneNode* node, T_GO* object) {
	node->renderable.push_back({ object, reinterpret_cast<MessageMethod>(&T_GO::Render) });

	return true;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
bool Scene::TryCreateDrawingGizmos(SceneNode* node, T_GO* object) {
	return false;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject> && DrawsGizmos<T_GO>
bool Scene::TryCreateDrawingGizmos(SceneNode* node, T_GO* object) {
	node->drawGizmos.push_back({ object, reinterpret_cast<MessageMethod>(&T_GO::DrawGizmos) });

	return true;
}

template<class T_GO, typename... T_Param>
	requires std::derived_from<T_GO, GameObject>
T_GO* Scene::CreateObjectOn(SceneNode* node, T_Param... params) {
	alignas(T_GO) unsigned char* dataBuf = new unsigned char[sizeof(T_GO)];
	memset(dataBuf, 0, sizeof(T_GO));
	volatile T_GO* bufAsObjPtr = reinterpret_cast<T_GO*>(dataBuf);

	bufAsObjPtr->node = node;

	T_GO* created = new(const_cast<T_GO*>(bufAsObjPtr)) T_GO(params...);

	created->node = node;
	created->runtimeTypeInfo = &typeid(T_GO);

	node->objects.push_back(created);

	TryCreateAwakeable(node, created);
	TryCreateEnableable(node, created);
	TryCreateDisableable(node, created);
	TryCreateUpdateable(node, created);
	TryCreateRenderable(node, created);
	TryCreateDrawingGizmos(node, created);

	for (SceneComponent* component : this->components) {
		GameObjectSystemBase* sys = dynamic_cast<GameObjectSystemBase*>(component);

		if (sys && sys->ValidObject(created)) {
			sys->RegisterObject(created);
		}
	}

	created->id = this->nextSceneNodeID++;

	created->enabled = true;

	return created;
}

template<class T_GO>
	requires std::derived_from<T_GO, GameObject>
std::vector<T_GO*> Scene::FindObjectsOfType() {
	return this->root->GetAllObjectsInChildren<T_GO>();
}

template<class T_SC>
	requires std::derived_from<T_SC, SceneComponent>
T_SC* Scene::GetComponent() {
	for (SceneComponent* component : this->components) {
		T_SC* result = dynamic_cast<T_SC*>(component);
		if (result) {
			return result;
	    }
	}

	return nullptr;
}

template<class T_SC>
	requires std::derived_from<T_SC, SceneComponent>
bool Scene::TryGetComponent(T_SC*& component) {
	component = GetComponent<T_SC>();

	return component != nullptr;
}

template<class T_SC>
	requires std::derived_from<T_SC, SceneComponent>
T_SC* Scene::GetOrCreateComponent() {
	T_SC* component = GetComponent<T_SC>();

	if (component == nullptr) {
		return AddComponent<T_SC>();
	}
}

template<class T_SC>
	requires std::derived_from<T_SC, SceneComponent>
T_SC* Scene::AddComponent() {
	T_SC* component = GetComponent<T_SC>();

	if (component == nullptr) {
		component = new T_SC(this);

		this->components.push_back(component);

		for (int i = this->components.size() - 2; i >= 0; i--) {
			if (this->components[i]->Order() > this->components[i + 1]->Order()) {
				std::swap(this->components[i], this->components[i + 1]);
			}
		}
	}

	return component;
}
