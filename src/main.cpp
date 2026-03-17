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
#include <Viewport.h>

class Mover : public GameObject, public ImGuiDrawable {
private:
	float pitch;
	float rotation;
	bool movementEnabled;
	int mode;
	float movementSpeed = 0.1f;
	float mouseSensitivity = 1.0f;
  Scene* starsScene;
public:
	Mover(Scene* starsScene) {
		this->pitch = 0;
		this->rotation = 0;
		this->mode = 0;
    this->starsScene = starsScene;
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

    if (GetScene()->Input()->KeyDown(Key::Delete)) {
      if (this->starsScene) {
        spdlog::info("Deleting starsScene");
        starsScene->GetRootNode()->GetParent()->DetachScene(starsScene);
        delete starsScene;
        ResourceDatabase::Global->FreeUnreferenced();
        this->starsScene = nullptr;
      } else {
        spdlog::info("starsScene already deleted");
      }
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
	ResourceRef<Mesh> starMesh;
  std::shared_ptr<Material> starMaterial;
	int starCount;
public:
	Stars(int starCount = 1000) {
		this->starMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/star.obj");
		
    std::shared_ptr<ShaderProgram> starProgram = ShaderProgram::Build()
		.WithVertexShader(
			ResourceDatabase::Global->Get<VertexShader>("./res/shaders/star.vert")
		).WithGeometryShader(
			ResourceDatabase::Global->Get<GeometryShader>("./res/shaders/star.geom")
		).WithPixelShader(
			ResourceDatabase::Global->Get<PixelShader>("./res/shaders/star.frag")
		).Link();
		starProgram->SetIgnoresDepthPrepass(true);
		starProgram->SetCastsShadows(false);

		this->starMaterial = std::make_shared<Material>(starProgram);
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
  std::shared_ptr<ShaderProgram> skyProg = ShaderProgram::Build().WithVertexShader(
		ResourceDatabase::Global->Get<VertexShader>("./res/shaders/skybox.vert")
	).WithPixelShader(
		ResourceDatabase::Global->Get<PixelShader>("./res/shaders/skybox.frag")
	).Link();

  std::shared_ptr<ShaderProgram> coloredProg = ShaderProgram::Build().WithVertexShader(
		ResourceDatabase::Global->Get<VertexShader>("./res/shaders/lit.vert")
	).WithPixelShader(
		ResourceDatabase::Global->Get<PixelShader>("./res/shaders/lambert color.frag")
	).Link();

  std::shared_ptr<ShaderProgram> diffuseTexProg = ShaderProgram::Build().WithVertexShader(
		ResourceDatabase::Global->Get<VertexShader>("./res/shaders/lit.vert")
	).WithPixelShader(
		ResourceDatabase::Global->Get<PixelShader>("./res/shaders/lambert.frag")
	).Link();

  std::shared_ptr<ShaderProgram> pbrProg = ShaderProgram::Build().WithVertexShader(
		ResourceDatabase::Global->Get<VertexShader>("./res/shaders/lit.vert")
	).WithPixelShader(
		ResourceDatabase::Global->Get<PixelShader>("./res/shaders/pbr.frag")
	).Link();

  std::shared_ptr<ShaderProgram> pbrRefractProg = ShaderProgram::Build().WithVertexShader(
		ResourceDatabase::Global->Get<VertexShader>("./res/shaders/lit.vert")
	).WithPixelShader(
		ResourceDatabase::Global->Get<PixelShader>("./res/shaders/pbr refract.frag")
	).Link();

	ResourceRef<Mesh> gmConstructMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/construct/construct.obj", true);
	ResourceRef<Mesh> cannonMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/cannon/cannon.obj");
	ResourceRef<Mesh> cubeMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/not_cube.obj");
	ResourceRef<Mesh> tvMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/tv_stand.fbx");
	ResourceRef<Mesh> schnozMesh = ResourceDatabase::Global->Get<Mesh>("./res/models/schnoz/schnoz.obj");

	ResourceRef<Cubemap> skyCubemap = ResourceDatabase::Global->Get<Cubemap>("./res/textures/citrus_orchard_road_puresky.hdr", Texture::HDRColorBuffer);
	skyCubemap->SetWrapModeU(TextureWrap::Clamp);
	skyCubemap->SetWrapModeV(TextureWrap::Clamp);
	skyCubemap->SetWrapModeW(TextureWrap::Clamp);

	ResourceRef<Texture2D> cannonDiffuse = ResourceDatabase::Global->Get<Texture2D>("./res/models/cannon/textures/cannon_01_diff_1k.png", Texture::ColorTextureRGB);
	ResourceRef<Texture2D> cannonNormal = ResourceDatabase::Global->Get<Texture2D>("./res/models/cannon/textures/cannon_01_nor_gl_1k.png", Texture::TechnicalMapXYZ);
	ResourceRef<Texture2D> cannonARM = ResourceDatabase::Global->Get<Texture2D>("./res/models/cannon/textures/cannon_01_arm_1k.png", Texture::TechnicalMapXYZ);
	
	ResourceRef<Texture2D> reflectiveDiffuse = ResourceDatabase::Global->Get<Texture2D>("./res/textures/material_preview/worn-shiny-metal-albedo.png", Texture::ColorTextureRGB);
	ResourceRef<Texture2D> reflectiveNormal = ResourceDatabase::Global->Get<Texture2D>("./res/textures/material_preview/worn-shiny-metal-Normal-ogl.png", Texture::TechnicalMapXYZ);
	ResourceRef<Texture2D> reflectiveARM = ResourceDatabase::Global->Get<Texture2D>("./res/textures/material_preview/worn-shiny-metal-arm.png", Texture::TechnicalMapXYZ);
	ResourceRef<Texture2D> roughARM = ResourceDatabase::Global->Get<Texture2D>("./res/textures/material_preview/worn-rough-metal-arm.png", Texture::TechnicalMapXYZ);
	ResourceRef<Texture2D> shinyNonMetalARM = ResourceDatabase::Global->Get<Texture2D>("./res/textures/material_preview/worn-shiny-nonmetal-arm.png", Texture::TechnicalMapXYZ);

	ResourceRef<Texture2D> schnozTexture = ResourceDatabase::Global->Get<Texture2D>("./res/models/schnoz/Diffuse.png", Texture::ColorTextureRGB);

	Viewport* schnozPreview = new Viewport();
	schnozPreview->GetFramebuffer()->CreateColorAttachment(true, false);
	schnozPreview->GetFramebuffer()->CreateDepthAttachment(false, false);
	schnozPreview->SetSize(glm::uvec2(1024, 512));

  std::shared_ptr<Material> cannonMat = std::make_shared<Material>(pbrProg);
	cannonMat->SetValue("albedoMap", cannonDiffuse);
	cannonMat->SetValue("normalMap", cannonNormal);
	cannonMat->SetValue("armMap", cannonARM);

  std::shared_ptr<Material> reflectiveMat = std::make_shared<Material>(pbrProg);
	reflectiveMat->SetValue("albedoMap", reflectiveDiffuse);
	reflectiveMat->SetValue("normalMap", reflectiveNormal);
	reflectiveMat->SetValue("armMap", reflectiveARM);

  std::shared_ptr<Material> roughMat = std::make_shared<Material>(pbrProg);
	roughMat->SetValue("albedoMap", reflectiveDiffuse);
	roughMat->SetValue("normalMap", reflectiveNormal);
	roughMat->SetValue("armMap", roughARM);

  std::shared_ptr<Material> shinyMat = std::make_shared<Material>(pbrRefractProg);
	shinyMat->SetValue("albedoMap", reflectiveDiffuse);
	shinyMat->SetValue("normalMap", reflectiveNormal);
	shinyMat->SetValue("armMap", reflectiveARM);

  std::shared_ptr<Material> skyMat = std::make_shared<Material>(skyProg);
	skyMat->SetValue("skyboxTexture", skyCubemap);

  std::shared_ptr<Material> tvMatStand = std::make_shared<Material>(coloredProg);
	tvMatStand->SetValue("uColor", glm::vec3(0.8, 0.8, 0.8));

  std::shared_ptr<Material> screenMat = std::make_shared<Material>(diffuseTexProg);
	screenMat->SetValue("uColor", glm::vec3(1, 1, 1));
	screenMat->SetValue("colorTex", (Texture2D*) schnozPreview->GetFramebuffer()->GetColorTexture());

  std::shared_ptr<Material> schnozMat = std::make_shared<Material>(diffuseTexProg);
	schnozMat->SetValue("uColor", glm::vec3(1, 1, 1));
	schnozMat->SetValue("colorTex", schnozTexture);

	auto constructNode = mainScene->CreateNode("gm_construct");
	constructNode->AddObject<MeshRenderer>(gmConstructMesh, gmConstructMesh->GetDefaultMaterials());

	auto cannonNode = mainScene->CreateNode("Cannon");
	cannonNode->AddObject<MeshRenderer>(cannonMesh, cannonMat);

	auto cubeNode = mainScene->CreateNode("Reflective Cube");
	cubeNode->AddObject<MeshRenderer>(cubeMesh, reflectiveMat);
	cubeNode->GlobalTransform().Position() = {-2.0f, 1.0f, 0.0f};
	cubeNode->GlobalTransform().Scale() = glm::vec3(0.6f);

	auto roughCubeNode = mainScene->CreateNode(cubeNode, "Rough Cube");
	roughCubeNode->AddObject<MeshRenderer>(cubeMesh, roughMat);
	roughCubeNode->LocalTransform().Position() = {0, 0, 3};

	auto shinyCubeNode = mainScene->CreateNode(cubeNode, "Shiny Cube");
	shinyCubeNode->AddObject<MeshRenderer>(cubeMesh, shinyMat);
	shinyCubeNode->LocalTransform().Position() = {0, 0, -3};

	auto cubeNode2 = mainScene->CreateNode("Reflective Cube");
	cubeNode2->AddObject<MeshRenderer>(cubeMesh, reflectiveMat);
	cubeNode2->GlobalTransform().Position() = {-25.0f, 1.0f, 0.0f};
	cubeNode2->GlobalTransform().Scale() = glm::vec3(0.6f);

	auto roughCubeNode2 = mainScene->CreateNode(cubeNode2, "Rough Cube");
	roughCubeNode2->AddObject<MeshRenderer>(cubeMesh, roughMat);
	roughCubeNode2->LocalTransform().Position() = {0, 0, 3};

	auto shinyCubeNode2 = mainScene->CreateNode(cubeNode2, "Shiny Cube");
	shinyCubeNode2->AddObject<MeshRenderer>(cubeMesh, shinyMat);
	shinyCubeNode2->LocalTransform().Position() = {0, 0, -3};

	auto cameraNode = mainScene->CreateNode("Camera");
	Camera* camera = cameraNode->AddObject<Camera>(Camera::Perspective(40.0f, 16.0f/9.0f, 0.5f, 200.0f));
	camera->LocalTransform().Position() = glm::vec3(0.0f, 1.5f, -10.0f);

	auto skyboxNode = mainScene->CreateNode(constructNode, "Floor");
	skyboxNode->AddObject<Skybox>(skyMat);

	auto lightNode = mainScene->CreateNode("Point Light");
	lightNode->AddObject<Light>(Light::PointLight({1, 1, 1}, 10, 2))->SetShadowCasting(true);
	lightNode->GlobalTransform().Position() = {-1, 2.2f, 0};

	auto lightNode2 = mainScene->CreateNode("Directional Light");
	lightNode2->AddObject<Light>(Light::DirectionalLight({1, 1, 1}, 2))->SetShadowCasting(true);
	lightNode2->GlobalTransform().Position() = {1, 2.2f, 0};
	lightNode2->GlobalTransform().Rotation() = glm::quat(glm::radians(glm::vec3(50.0f, -20.0f, 0.0f)));

	auto envProbe = mainScene->CreateNode(cubeNode, "Reflection Probe");
	envProbe->AddObject<ReflectionProbe>();

	auto envProbe2 = mainScene->CreateNode("Reflection Probe");
	envProbe2->AddObject<ReflectionProbe>();
	envProbe2->GlobalTransform().Position() = {-10.0f, 1.5f, 0.6f};

	auto envProbe3 = mainScene->CreateNode("Reflection Probe");
	envProbe3->AddObject<ReflectionProbe>();
	envProbe3->GlobalTransform().Position() = {-29.0f, 1.5f, 0.6f};

	auto envProbe4 = mainScene->CreateNode(shinyCubeNode, "Reflection Probe");
	envProbe4->AddObject<ReflectionProbe>();
	
	auto starsAttachmentNode = mainScene->CreateNode("Stars Scene Attachment");
	
	auto starsScene = new Scene();

	auto starsNode = starsScene->CreateNode("Stars");
	starsNode->AddObject<Stars>(1000);
	starsNode->GlobalTransform().Position() = {-15.0f, 5.5f, -105.0f};

	starsAttachmentNode->AttachScene(starsScene);

	cameraNode->AddObject<Mover>(starsScene);

	SceneNode* tvNode = mainScene->CreateNode("TV");
	tvNode->LocalTransform().Scale() = glm::vec3(1.5, 1.5, 1.5);
	tvNode->LocalTransform().Position() = glm::vec3(3, 0, -2);
	tvNode->LocalTransform().Rotation() = glm::quat(glm::radians(glm::vec3(-90.0f, 20.0f, 0.0f)));

	auto tvRenderer = tvNode->AddObject<MeshRenderer>(tvMesh, nullptr);
	tvRenderer->SetMaterial(tvMatStand, 0);
	tvRenderer->SetMaterial(screenMat, 1);
	tvRenderer->SetMaterial(tvMatStand, 2);
	tvRenderer->SetMaterial(tvMatStand, 3);

	SceneNode* schnozCameraNode = mainScene->CreateNode("Schnoz Camera");
	schnozCameraNode->LocalTransform().Position() = glm::vec3(-56.5, 2.0, -2.0);
	schnozCameraNode->LocalTransform().Rotation() = glm::quat(glm::radians(glm::vec3(5.0f, 85.0f, 0.0f)));
	
	auto schnozCamera = schnozCameraNode->AddObject<Camera>(Camera::Perspective(40.0f, 16.0f/9.0f, 0.5f, 200.0f));
	schnozCamera->SetAspectRatio(2);
	schnozCamera->SetRenderTarget(schnozPreview);
	schnozCamera->SetLayerMask(uint8_t(5));

	SceneNode* schnozNode = mainScene->CreateNode("Schnoz");
	schnozNode->LocalTransform().Position() = glm::vec3(-53.5, 1.75, -2.4);
	schnozNode->LocalTransform().Scale() = glm::vec3(0.15, 0.15, 0.15);
	schnozNode->AddObject<MeshRenderer>(schnozMesh, schnozMat);
	schnozNode->AddObject<AutoRotator>(1);
	schnozNode->SetLayer(5);

	SceneNode* schnozLightNode = mainScene->CreateNode("Schnoz Light");
	schnozLightNode->LocalTransform().Position() = glm::vec3(-55.5, 3.0, -2.0);
	schnozLightNode->AddObject<Light>(Light::PointLight(glm::vec3(1, 1, 1), 5, 5));

	cameraNode->AddObject<Bloom>();
	cameraNode->AddObject<Tonemapper>()->SetOperator(Tonemapper::TonemapperOperator::GranTurismo);

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
