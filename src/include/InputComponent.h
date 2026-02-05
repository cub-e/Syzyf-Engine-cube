#pragma once

#include "SceneComponent.h"
#include "events/Event.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <string>
#include <vector>
#include "Scene.h"

using EventFactory = std::function<std::unique_ptr<Event>()>;

class InputComponent : public SceneComponent {
public:
  InputComponent(Scene* scene);
  virtual ~InputComponent();

  template <typename T_Event, typename... Args>
  void BindAction(int key, Args&&... args);

  void OnPreUpdate();
private:
  // Should perhaps allow for storing different maps and switch them depending on the context eg. opening a menu
  //  but maybe it's fine as is, godot doesn't differentiate between different contexts
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
