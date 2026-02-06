#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#include "SceneComponent.h"

class Scene;
class Event;
struct GLFWwindow;

using EventFactory = std::function<std::unique_ptr<Event>()>;

class InputComponent : public SceneComponent {
public:
  InputComponent(Scene* scene);
  virtual ~InputComponent();

  template <typename T_Event, typename... Args>
  void BindAction(int key, Args&&... args);

  void OnPreUpdate();
private:
  GLFWwindow* m_ownerWindow = nullptr;
  std::unordered_map<int, std::vector<EventFactory>> inputMap;
public:
  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  void SetGlfwCallbacks(GLFWwindow* window);
  static void DispatchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void DispatchMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void DispatchScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void DispatchCharCallback(GLFWwindow* window, unsigned int codepoint);
};

template <typename T_Event, typename... Args>
void InputComponent::BindAction(int key, Args&&... args) {
  inputMap[key].push_back([=]() {
    return std::make_unique<T_Event>(args...);
  });
}
