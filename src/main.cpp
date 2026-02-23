#include "GltfImporter.h"
#include "imgui.h"

#include <Formatters.h>
#include <Shader.h>
#include <Mesh.h>
#include <Material.h>
#include <MeshRenderer.h>
#include <Scene.h>
#include <Graphics.h>
#include <Camera.h>
#include <Skybox.h>
#include <Resources.h>
#include <Light.h>
#include <Bloom.h>
#include <ReflectionProbe.h>
#include <ReflectionProbeSystem.h>
#include <Tonemapper.h>
#include <Debug.h>
#include <InputSystem.h>
#include <Engine.h>

class Mover : public GameObject, public ImGuiDrawable {
private:
  float pitch;
  float rotation;
  bool movementEnabled;
  int mode;
  float movementSpeed = 0.1f;
  float mouseSensitivity = 1.0f;
public:
  Mover() {
    this->pitch = 0;
    this->rotation = 0;
    this->mode = 0;
  }

  void Update() {
    if (movementEnabled) {
      glm::vec3 movement = glm::zero<glm::vec3>();
      glm::quat rotation = glm::identity<glm::quat>();

      glm::vec3 right = this->GlobalTransform().Right();
      glm::vec3 up = glm::vec3(0, 1, 0);
      glm::vec3 forward = mode == 0 ? glm::cross(right, up) : this->GlobalTransform().Forward();

      if (GetScene()->Input()->KeyPressed(Key::A)) {
        movement += right;
      }
      if (GetScene()->Input()->KeyPressed(Key::D)) {
        movement -= right;
      }
      if (GetScene()->Input()->KeyPressed(Key::W)) {
        movement += forward;
      }
      if (GetScene()->Input()->KeyPressed(Key::S)) {
        movement -= forward;
      }
  
      glm::vec2 deltaMovement = GetScene()->Input()->GetMouseMovement();

      this->rotation -= (deltaMovement.x / 20) * this->mouseSensitivity;
      this->pitch -= (deltaMovement.y / 20) * this->mouseSensitivity;

      if (this->rotation < -180) {
        this->rotation += 360;
      }
      else if (this->rotation > 180) {
        this->rotation -= 360;
      }

      this->pitch = glm::clamp(this->pitch, -89.0f, 89.0f);
      this->GlobalTransform().Position() += movement * this->movementSpeed;
      this->GlobalTransform().Rotation() = glm::angleAxis(
        glm::radians(this->rotation), glm::vec3(0, 1, 0)
      ) * glm::angleAxis(glm::radians(this->pitch), glm::vec3(1, 0, 0));
    }

    if (GetScene()->Input()->KeyDown(Key::Escape)) {
      this->movementEnabled = !this->movementEnabled;

      GetScene()->Input()->SetMouseLocked(this->movementEnabled);
    }
  }

  virtual void DrawImGui() {
    const char* modes[] { "Walking", "Freecam", };

    ImGui::Combo("Movement type", &this->mode, modes, 2);

    ImGui::InputFloat("Movement speed", &this->movementSpeed);
    ImGui::InputFloat("Mouse sensitivity", &this->mouseSensitivity);
  }
};

class AutoRotator : public GameObject {
private:
  float speed;
public:
  AutoRotator(float speed) {
    this->speed = speed;
  }

  void Update() {
    glm::quat rotation = glm::angleAxis(glm::radians(this->speed), glm::vec3(0.0f, 1.0f, 0.0f));

    this->LocalTransform().Rotation() *= rotation;
  }
};

class Stars : public GameObject, public ImGuiDrawable {
private:
  Mesh* starMesh;
  Material* starMaterial;
  int starCount;
public:
  Stars(int starCount = 1000) {
    this->starMesh = GetScene()->Resources()->Get<Mesh>("./res/models/star.obj");
    
    ShaderProgram* starProgram = ShaderProgram::Build()
    .WithVertexShader(
      GetScene()->Resources()->Get<VertexShader>("./res/shaders/star.vert")
    ).WithGeometryShader(
      GetScene()->Resources()->Get<GeometryShader>("./res/shaders/star.geom")
    ).WithPixelShader(
      GetScene()->Resources()->Get<PixelShader>("./res/shaders/star.frag")
    ).Link();
    starProgram->SetIgnoresDepthPrepass(true);
    starProgram->SetCastsShadows(false);

    this->starMaterial = new Material(starProgram);
    this->starCount = starCount;
  }

  void Render() {
    GetScene()->GetGraphics()->DrawMeshInstanced(
      this->starMesh,
      0,
      this->starMaterial,
      this->GlobalTransform(),
      this->starCount,
      BoundingBox::CenterAndExtents(glm::vec3(0, 0, 0), glm::vec3(15, 15, 15))
    );
  }

  void DrawImGui() {
    ImGui::InputInt("Star count", &this->starCount);
  }
};

void InitScene(Scene* mainScene) {
	ShaderProgram* skyProg = ShaderProgram::Build().WithVertexShader(
		mainScene->Resources()->Get<VertexShader>("./res/shaders/skybox.vert")
	).WithPixelShader(
		mainScene->Resources()->Get<PixelShader>("./res/shaders/pbr_gltf.frag")
	).Link();
  auto* tvsGltfImporterNode = GltfImporter::LoadScene(mainScene, "./res/models/tvs/tvs.glb", pbrGltfProg, "tvs");
  auto* cameraCubeNode = GltfImporter::LoadScene(mainScene, "./res/models/camera_cube.glb", pbrGltfProg, "cameraCube");
  cameraCubeNode->GlobalTransform().Position().SetZ(-5.0f);

  mainScene->AddComponent<DebugInspector>();
}

int main(int, char**) {
	if (!Engine::Setup(InitScene)) {
		spdlog::error("Failed to initialize project!");
		return EXIT_FAILURE;
	}

	spdlog::info("Initialized project.");

	Engine::MainLoop();

  return 0;
}
