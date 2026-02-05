#pragma once

#include <SceneComponent.h>
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
};
