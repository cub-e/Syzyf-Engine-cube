#include <GameObject.h>

#include <Scene.h>

Transform& GameObject::GetTransform() {
	return this->node->GetTransform();
}

template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
T_GO* GameObject::AddObject() {
	return this->node->AddObject<T_GO>();
}