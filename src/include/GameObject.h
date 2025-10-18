#pragma once

#include <concepts>

class SceneNode;
class Scene;
class Transform;

class GameObject {
	friend class Scene;
private:
	SceneNode* node;
public:
	Transform& GetTransform();

	template<class T_GO>
		requires std::derived_from<T_GO, GameObject>
	T_GO* AddObject();
};

// template <class T_Required>
// 	requires std::derived_from<T_Required, GameObject>
// class Requires { };