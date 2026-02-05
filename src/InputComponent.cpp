#include "InputComponent.h"
#include <unordered_map>
#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"

InputComponent::InputComponent(Scene* scene): SceneComponent(scene) {};
InputComponent::~InputComponent() {};

void InputComponent::SetInputMap(const std::unordered_map<int, std::vector<std::string>> inputMap) {
  this->inputMap = inputMap;
}

void InputComponent::OnPreUpdate() {
}

void InputComponent::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

  if (action == GLFW_PRESS) {
    if (inputMap.contains(key)) {
      auto actions = inputMap[key];
      for (auto action : actions) {
        spdlog::info("Action: {} pressed", action);
      }
    }
  }
}

void InputComponent::SetGlfwCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, InputComponent::DispatchKeyCallback);
    glfwSetMouseButtonCallback(window, InputComponent::DispatchMouseButtonCallback);
    glfwSetScrollCallback(window, InputComponent::DispatchScrollCallback);
    glfwSetCharCallback(window, InputComponent::DispatchCharCallback);
}

void InputComponent::DispatchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    InputComponent* instance = static_cast<InputComponent*>(glfwGetWindowUserPointer(window));
    if (instance) {
        instance->KeyCallback(window, key, scancode, action, mods);
    }
}
void InputComponent::DispatchMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;
}
void InputComponent::DispatchScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;
}
void InputComponent::DispatchCharCallback(GLFWwindow* window, unsigned int codepoint) {
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantCaptureKeyboard) return;
}
