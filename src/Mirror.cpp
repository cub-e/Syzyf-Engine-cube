#include "Mirror.h"

#include "Camera.h"
#include "Shader.h"
#include "Material.h"
#include "Framebuffer.h"
#include "MeshRenderer.h"

#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_inverse.hpp>

class Mover : public GameObject {};

Mirror::Mirror(Mesh* mesh) {
  SceneNode* rootNode = this->GetScene()->CreateNode(this->GetNode());
  this->cameraNode = this->GetScene()->CreateNode(rootNode);
  cameraNode->AddObject<Camera>(Camera::Perspective(
    40.0f, 16.0f/9.0f, 0.5f, 200.0f));
  
  this->viewport = Viewport();
  this->viewport.GetFramebuffer()->CreateColorAttachment(true, false);
  this->viewport.GetFramebuffer()->CreateDepthAttachment(true, false);
  this->viewport.SetSize(glm::uvec2(1920, 1080));

  this->cameraNode->GetObject<Camera>()->SetAspectRatio(16.0f / 9.0f);
  this->cameraNode->GetObject<Camera>()->SetRenderTarget(&this->viewport);

  ShaderProgram* shaderProgram = ShaderProgram::Build().WithVertexShader(
    this->GetScene()->Resources()->Get<VertexShader>("./res/shaders/mirror.vert")
  ).WithPixelShader(
    this->GetScene()->Resources()->Get<PixelShader>("./res/shaders/mirror.frag")
  ).Link();

  this->material = new Material(shaderProgram);
  this->material->SetValue("uColor", glm::vec3(1, 1, 1));
  this->material->SetValue("colorTex", (Texture2D*) this->viewport.GetFramebuffer()->GetColorTexture());

  this->GetNode()->AddObject<MeshRenderer>(mesh, this->material);
  this->GetNode()->GetTransform().GlobalTransform().Scale() = glm::vec3(10.0f);
};

void Mirror::Update() {
  if (this->playerNode == nullptr) {
    this->playerNode = this->GetScene()->FindObjectsOfType<Mover>().front()->GetNode(); // xd
    if (this->playerNode != nullptr) {
      spdlog::info("Mirror: Found player camera node");
    } else {
      spdlog::warn("Mirror: player camera node is missing");
      return;
    }
  }

  const glm::mat4& playerCameraTransform = this->playerNode->GlobalTransform().Value();
  const glm::mat4& transform = this->GlobalTransform().Value();
  glm::mat4 relativeTransform = glm::affineInverse(transform) * playerCameraTransform;
  relativeTransform[3][2] = -relativeTransform[3][2]; // Z translation

  glm::vec3 cameraPosition = glm::vec3(relativeTransform[3]);

  // -transform.basis.z
  glm::vec3 localForward = -glm::vec3(
    relativeTransform[2][0],
    relativeTransform[2][1],
    relativeTransform[2][2]
  );
  // transform.basis.y
  glm::vec3 localUp = glm::vec3(
    relativeTransform[1][0],
    relativeTransform[1][1],
    relativeTransform[1][2]
  );

  localForward.z = -localForward.z;
  localUp.z = -localUp.z;

  glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f), localForward, localUp);
  glm::mat4 reflectedTransform = glm::mat3(glm::inverse(viewMatrix)); 
  reflectedTransform[3] = glm::vec4(cameraPosition, 1.0f);

  this->cameraNode->GlobalTransform() = transform * reflectedTransform;
}
