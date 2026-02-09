#include "physics/PhysicsDebugRenderer.h"
#include "Camera.h"
#include "Resources.h"
#include "Shader.h"
#include "Scene.h"
#include <glm/gtc/type_ptr.hpp>

MyDebugRenderer::MyDebugRenderer(Scene *scene) : SceneComponent(scene) {
    shader = ShaderProgram::Build()
        .WithVertexShader(GetScene()->Resources()->Get<VertexShader>("./res/shaders/physics_debug/physics_debug.vert"))
        .WithPixelShader(GetScene()->Resources()->Get<PixelShader>("./res/shaders/physics_debug/physics_debug.frag"))
        .Link();
    
    if (shader) {
        shader->SetCastsShadows(false);
        shader->SetIgnoresDepthPrepass(true);
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(6);

    glBindVertexArray(0);
}

void MyDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    JPH::Vec4 color = inColor.ToVec4();

    lines.push_back({
        (float)inFrom.GetX(), (float)inFrom.GetY(), (float)inFrom.GetZ(),
        color.GetX(), color.GetY(), color.GetZ()
    });

    lines.push_back({
        (float)inTo.GetX(), (float)inTo.GetY(), (float)inTo.GetZ(),
        color.GetX(), color.GetY(), color.GetZ()
    });
}

void MyDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) {}

void MyDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) {}

void MyDebugRenderer::Render() {
    if (lines.empty() || !shader) return;

    GLuint shaderHandle = shader->GetHandle();
    glUseProgram(shaderHandle);

    std::vector<Camera*> cameraObjects = GetScene()->FindObjectsOfType<Camera>();

    if (cameraObjects.empty()) return;

    glm::mat4 vp = cameraObjects.front()->ViewProjectionMatrix();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(DebugVertex), lines.data(), GL_STREAM_DRAW);

    glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());

    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    lines.clear();
}
