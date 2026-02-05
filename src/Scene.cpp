#include <Scene.h>

#include <algorithm>
#include <stack>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <spdlog/spdlog.h>
#include <GameObject.h>
#include <Graphics.h>

SceneNode::SceneNode(Scene* scene) :
scene(scene),
transform(),
children(),
parent(nullptr),
enabled(true),
updateable(),
renderable(),
drawGizmos(),
name("") {
	this->transform.parent = this;
}

SceneNode::~SceneNode() {
	int objectsCount = this->objects.size();
	GameObject* objectsCopy[objectsCount];

	std::copy(this->objects.begin(), this->objects.end(), objectsCopy);

	for (int i = 0; i < objectsCount; i++) {
		delete objectsCopy[i];
	}

	this->SetParent(nullptr);

	int childrenCount = this->children.size();
	SceneNode* childrenCopy[childrenCount];

	std::copy(this->children.begin(), this->children.end(), childrenCopy);

	for (int i = 0; i < childrenCount; i++) {
		delete childrenCopy[i];
	}

	this->scene->DeleteNodeInternal(this);
}

void SceneNode::RecalculateTransform() {
	if (this->transform.LocalTransform().IsDirty()) {
		if (this->parent) {
			this->transform.GlobalTransform() = this->parent->GlobalTransform().Value() * this->transform.LocalTransform().Value();
		}
		else {
			this->transform.GlobalTransform() = this->transform.LocalTransform().Value();
		}
	}
	else if (this->transform.GlobalTransform().IsDirty()) {
		if (this->parent) {
			this->transform.LocalTransform() = this->transform.GlobalTransform().Value() * glm::inverse(this->parent->GlobalTransform().Value());
		}
		else {
			this->transform.LocalTransform() = this->transform.GlobalTransform().Value();
		}
	}

	this->transform.ClearDirty();
}

SceneTransform& SceneNode::GetTransform() {
	if (this->transform.IsDirty()) {
		RecalculateTransform();
	}

	return this->transform;
}

SceneTransform::TransformAccess& SceneNode::LocalTransform() {
	return this->GetTransform().LocalTransform();
}

SceneTransform::TransformAccess& SceneNode::GlobalTransform() {
	return this->GetTransform().GlobalTransform();
}

int SceneNode::GetID() const {
	return this->id;
}

std::string SceneNode::GetName() const {
	return this->name;
}
void SceneNode::SetName(const std::string& name) { 
	this->name = name;
}

Scene* SceneNode::GetScene() {
	return this->scene;
}

bool SceneNode::IsEnabled() const {
	return this->enabled;
}
void SceneNode::SetEnabled(bool value) {
	this->enabled = value;
}

const std::vector<SceneNode*> SceneNode::GetChildren() {
	return this->children;
}

SceneNode* SceneNode::GetParent() {
	return this->parent;
}

void SceneNode::SetParent(SceneNode* newParent) {
	if (this->parent) {
		auto posInParentChildren = std::find(this->parent->children.begin(), this->parent->children.end(), this);
		if (posInParentChildren != this->parent->children.end()) {
			this->parent->children.erase(posInParentChildren);
		}
	}

	this->parent = newParent;

	if (this->parent) {
		this->parent->children.push_back(this);
	}
}

bool SceneNode::IsChildOf(const SceneNode* node) {
	if (this->parent == node) {
		return true;
	}
	return this->parent->IsChildOf(node);
}

void SceneNode::MarkDirty() {
	this->transform.LocalTransform().MarkDirty();

	MarkChildrenDirty();
}

void SceneNode::MarkChildrenDirty() {
	for (auto child : this->children) {
		child->MarkDirty();
	}
}

const std::vector<GameObject*> SceneNode::AttachedObjects() {
	return this->objects;
}

void SceneNode::DeleteObject(GameObject* obj) {
	this->objects.erase(std::find(this->objects.begin(), this->objects.end(), obj));

	this->scene->DeleteObjectInternal(obj);
}

Scene::Scene() :
root(nullptr),
nextSceneNodeID(0),
nextGameObjectID(0) {
	this->root = CreateNode("root");
	this->graphics = AddComponent<SceneGraphics>();
}

void SceneNode::MessageReceiver::Message() {
	(*this->objPtr.*this->methodPtr)();
}

void Scene::DeleteObjectInternal(GameObject* obj) {
	SceneNode* node = obj->node;

	std::erase_if(node->updateable, [obj](const auto& msgRcvr) {
		return msgRcvr.objPtr == obj;
	} );

	std::erase_if(node->renderable, [obj](const auto& msgRcvr) {
		return msgRcvr.objPtr == obj;
	} );

	for (auto* component : this->components) {
		GameObjectSystemBase* componentAsSystem = dynamic_cast<GameObjectSystemBase*>(component);

		if (componentAsSystem) {
			componentAsSystem->UnregisterObjectForced(obj);
		}
	}
}

void Scene::DeleteNodeInternal(SceneNode* node) {
	if (node == this->root) {
		this->root = CreateNode("root");
	}
}

SceneNode* Scene::CreateNode() {
	return CreateNode(this->root, "");
}
SceneNode* Scene::CreateNode(SceneNode* parent) {
	return CreateNode(parent, "");
}

SceneNode* Scene::CreateNode(const std::string& name) {
	return CreateNode(this->root, name);
}
SceneNode* Scene::CreateNode(SceneNode* parent, const std::string& name) {
	SceneNode* result = new SceneNode(this);

	if (parent) {
		result->SetParent(parent);
	}
	else if (this->root) {
		result->SetParent(this->root);
	}
	else {
		this->root = result;
	}

	result->id = this->nextSceneNodeID++;
	result->name = name;

	return result;
}

SceneGraphics* Scene::GetGraphics() {
	return this->graphics;
}

SceneNode* Scene::GetRootNode() {
	return this->root;
}

void Scene::DeleteObject(GameObject* obj) {
	delete obj;
}

void Scene::DeleteNode(SceneNode* node) {
	delete node;
}

void Scene::Update() {
	for (auto& component: this->components) {
		component->OnPreUpdate();
	}

	std::stack<SceneNode*> nodeStack;
	std::stack<SceneNode::MessageReceiver*> updateables;

	nodeStack.push(this->root);

	while (!nodeStack.empty()) {
		SceneNode* top = nodeStack.top();
		nodeStack.pop();

		if (!top->enabled) {
			continue;
		}

		for (SceneNode* child : top->GetChildren()) {
			nodeStack.push(child);
		}

		for (SceneNode::MessageReceiver& msg : top->updateable) {
			updateables.push(&msg);
		}
	}

	while (!updateables.empty()) {
		
		updateables.top()->Message();
		updateables.pop();
	}

	for (auto& component: this->components) {
		component->OnPostUpdate();
	}
}

void Scene::Render() {
	for (auto& component: this->components) {
		component->OnPreRender();
	}

	std::stack<SceneNode*> nodeStack;
	std::stack<SceneNode::MessageReceiver*> renderables;
	std::stack<SceneNode::MessageReceiver*> gizmos;

	nodeStack.push(this->root);

	while (!nodeStack.empty()) {
		SceneNode* top = nodeStack.top();
		nodeStack.pop();

		if (!top->enabled) {
			continue;
		}

		for (SceneNode* child : top->GetChildren()) {
			nodeStack.push(child);
		}

		for (SceneNode::MessageReceiver& msg : top->renderable) {
			renderables.push(&msg);
		}

		for (SceneNode::MessageReceiver& msg : top->drawGizmos) {
			gizmos.push(&msg);
		}
	}

	while (!renderables.empty()) {
		renderables.top()->Message();
		renderables.pop();
	}

	while (!gizmos.empty()) {
		gizmos.top()->Message();
		gizmos.pop();
	}

	for (auto& component: this->components) {
		component->OnPostRender();
	}
}


void Scene::DrawImGui() {
	for (auto& component: this->components) {
		component->DrawImGui();
	}
}
