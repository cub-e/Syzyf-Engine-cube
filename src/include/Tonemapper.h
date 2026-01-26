#pragma once

#include <PostProcessEffect.h>
#include <glad/glad.h>

class ComputeShaderDispatch;

class Tonemapper : public PostProcessEffect {
public:
	enum class TonemapperFunction {
		None,
		Reinhard,
		Aces,
		GranTurismo,
	};
private:
	TonemapperFunction function;

	ComputeShaderDispatch* reinhardTonemapperShader;
	ComputeShaderDispatch* acesTonemapperShader;
	ComputeShaderDispatch* gtTonemapperShader;
public:
	Tonemapper();

	TonemapperFunction GetCurrentFunction() const;

	void SetFunction(TonemapperFunction func);

	virtual void OnPostProcess(const PostProcessParams* params);
};