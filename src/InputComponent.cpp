#include "InputComponent.h"
#include <unordered_map>

InputComponent::InputComponent(Scene* scene): SceneComponent(scene) {};
InputComponent::~InputComponent() {};

void InputComponent::SetInputMap(const std::unordered_map<std::string, std::vector<int>> inputMap) {
  this->inputMap = inputMap;
}

void InputComponent::OnPreUpdate() {
}

void InputComponent::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  
  if (action == GLFW_PRESS) {
    if (auto actions = inputMap.find(key); actions != inputMap.end()) {
      for (auto action : actions) {
        spdlog::info("Action: {} pressed")
      }
    }
  }
}
