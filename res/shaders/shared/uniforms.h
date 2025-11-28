#ifndef SHADER_UNIFORMS_H

#ifdef __cplusplus

#pragma once

#include <glm/glm.hpp>

#define mat4 glm::mat4
#define UNIFORM_DECL(bindingPoint) struct

#else

#define UNIFORM_DECL(bindingPoint) layout (std140, binding = bindingPoint) uniform

#endif

UNIFORM_DECL(0) ShaderGlobalUniforms
{
	mat4 Global_ViewMatrix;
	mat4 Global_ProjectionMatrix;
	mat4 Global_VPMatrix;
	float Global_Time;
};
UNIFORM_DECL(1) ShaderObjectUniforms
{
	mat4 Object_ModelMatrix;
	mat4 Object_MVPMatrix;
};

#ifdef mat4
#undef mat4
#endif

#ifdef UNIFORM_DECL
#undef UNIFORM_DECL
#endif

#define SHADER_UNIFORMS_H
#endif