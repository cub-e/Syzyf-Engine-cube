#pragma once

#include "SceneComponent.h"
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <vector>
#include <glad/glad.h> 

class ShaderProgram;

class MyDebugRenderer : public JPH::DebugRendererSimple, public SceneComponent
{
public:
  MyDebugRenderer(Scene *scene);
  virtual ~MyDebugRenderer() = default;

  virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
  virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;
  virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override;

  void Render();

private:
  struct DebugVertex {
    float x, y, z;
    float r, g, b;
  };

  ShaderProgram* shader;
  GLuint vao = 0;
  GLuint vbo = 0;
  std::vector<DebugVertex> lines;
};
