#pragma once

#include "SceneComponent.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <string>
#include <vector>

class InputComponent : public SceneComponent {
public:
  InputComponent(Scene* scene);
  virtual ~InputComponent();

  void OnPreUpdate();
private:
  // Should perhaps allow for storing different maps and switch them depending on the context eg. opening a menu
  //  but maybe it's fine as is, godot doesn't differentiate between different contexts
  std::unordered_map<int, std::vector<std::string>> inputMap;
public:
  void SetInputMap(const std::unordered_map<int, std::vector<std::string>> inputMap);
  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  void SetGlfwCallbacks(GLFWwindow* window);
  static void DispatchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void DispatchMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void DispatchScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void DispatchCharCallback(GLFWwindow* window, unsigned int codepoint);
};
