#pragma once

#include <filesystem>
#include <concepts>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "stb_image.h"

namespace fs = std::filesystem;

enum class TextureType {
	Texture2D,
	Cubemap
};

enum class TextureFormat {
	Grayscale = 1,
	RG,
	RGB,
	RGBA
};

class Texture {
private:
	template<typename T>
	struct TextureInfoBit {
		T value;
		bool dirty;
	};

	GLuint handle;
	bool dirty;

	TextureInfoBit<GLenum> wrapU;
	TextureInfoBit<GLenum> wrapV;
	TextureInfoBit<GLenum> minFilter;
	TextureInfoBit<GLenum> magFilter;
	TextureInfoBit<bool> mipmapped;
public:
	template <class T_Tex>
	static T_Tex* Load(fs::path texturePath, TextureFormat format) = delete;

	virtual constexpr TextureType GetType() const = 0;

	GLuint GetHandle();

	GLenum GetWrapModeU() const;
	void SetWrapModeU(GLenum wrapMode);
	GLenum GetWrapModeV() const;
	void SetWrapModeV(GLenum wrapMode);

	GLenum GetMinFilter() const;
	void SetMinFilter(GLenum minFilter);
	GLenum GetMagFilter() const;
	void SetMagFilter(GLenum magFilter);

	bool HasMipmaps() const;
	void GenerateMipmaps();

	bool IsDirty() const;
	void Update();
};

class Texture2D : public Texture {
public:
	virtual constexpr TextureType GetType() const {
		return TextureType::Texture2D;
	}
};

class Cubemap : public Texture {
public:
	virtual constexpr TextureType GetType() const {
		return TextureType::Cubemap;
	}
};

template<> Texture2D* Texture::Load<Texture2D>(fs::path texturePath, TextureFormat format);