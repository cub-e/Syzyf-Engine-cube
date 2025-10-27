#define STB_IMAGE_IMPLEMENTATION

#include <Texture.h>

bool WrapModeValid(GLenum wrapMode) {
	return (
		wrapMode == GL_REPEAT
		||
		wrapMode == GL_MIRRORED_REPEAT
		||
		wrapMode == GL_CLAMP_TO_EDGE
	);
}

bool MinFilterValid(GLenum minFilter) {
	return (
		minFilter == GL_NEAREST
		||
		minFilter == GL_LINEAR
		||
		minFilter == GL_NEAREST_MIPMAP_NEAREST
		||
		minFilter == GL_LINEAR_MIPMAP_NEAREST
		||
		minFilter == GL_NEAREST_MIPMAP_LINEAR
		||
		minFilter == GL_LINEAR_MIPMAP_LINEAR
	);
}

bool MagFilterValid(GLenum magFilter) {
	return (
		magFilter == GL_NEAREST
		||
		magFilter == GL_LINEAR
	);
}

GLuint Texture::GetHandle() {
	return this->handle;
}

GLenum Texture::GetWrapModeU() const {
	return this->wrapU.value;
}
void Texture::SetWrapModeU(GLenum wrapMode) {
	if (this->wrapU.value != wrapMode && WrapModeValid(wrapMode)) {
		this->wrapU.value = wrapMode;
		
		this->wrapU.dirty = true;
		this->dirty = true;
	}
}
GLenum Texture::GetWrapModeV() const {
	return this->wrapV.value;
}
void Texture::SetWrapModeV(GLenum wrapMode) {
	if (this->wrapV.value != wrapMode && WrapModeValid(wrapMode)) {
		this->wrapV.value = wrapMode;
		
		this->wrapV.dirty = true;
		this->dirty = true;
	}
}

GLenum Texture::GetMinFilter() const {
	return this->minFilter.value;
}
void Texture::SetMinFilter(GLenum minFilter) {
	if (this->minFilter.value != minFilter && MinFilterValid(minFilter)) {
		this->minFilter.value = minFilter;
		
		this->minFilter.dirty = true;
		this->dirty = true;
		
		if (minFilter >= GL_NEAREST_MIPMAP_NEAREST) {
			this->GenerateMipmaps();
		}
	}
}
GLenum Texture::GetMagFilter() const {
	return this->magFilter.value;
}
void Texture::SetMagFilter(GLenum magFilter) {
	if (this->magFilter.value != magFilter && MagFilterValid(magFilter)) {
		this->magFilter.value = magFilter;
		
		this->magFilter.dirty = true;
		this->dirty = true;
	}
}

bool Texture::HasMipmaps() const {
	return this->mipmapped.value;
}
void Texture::GenerateMipmaps() {
	if (!this->mipmapped.value) {
		this->mipmapped.value = true;
		
		this->mipmapped.dirty = true;
		this->dirty = true;
	}
}

bool Texture::IsDirty() const {
	return this->dirty;
}
void Texture::Update() {
	GLenum glTexType[] {
		GL_TEXTURE_2D,
		// GL_TEXTURE_CUBE_MAP
	};

	if (!this->dirty) {
		return;
	}

	GLenum type = glTexType[(int) this->GetType()];

	glBindTexture(type, this->handle);

	if (this->mipmapped.dirty) {
		glGenerateMipmap(type);

		this->mipmapped.dirty = false;
	}
	if (this->wrapU.dirty) {
		glTexParameteri(type, GL_TEXTURE_WRAP_S, this->wrapU.value);

		this->wrapU.dirty = false;
	}
	if (this->wrapV.dirty) {
		glTexParameteri(type, GL_TEXTURE_WRAP_T, this->wrapV.value);

		this->wrapV.dirty = false;
	}
	if (this->minFilter.dirty) {
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, this->minFilter.value);

		this->minFilter.dirty = false;
	}
	if (this->magFilter.dirty) {
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, this->magFilter.value);

		this->magFilter.dirty = false;
	}

	this->dirty = false;

	glBindTexture(type, 0);
}

template<> Texture2D* Texture::Load<Texture2D>(fs::path texturePath, TextureFormat format) {
	static GLenum glFormats[] {
		0,
		GL_RED,
		GL_RG,
		GL_RGB,
		GL_RGBA
	};

	fs::directory_entry textureFile(texturePath);

	if (!textureFile.exists() || !textureFile.is_regular_file()) {
		spdlog::error("File {} doesn't exist", texturePath.string());
		return nullptr;
	}

	int width, height, nrChannels;
	unsigned char *textureData = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

	if (!textureData) {
		spdlog::error("stbi_load failed on file {}", texturePath.string());
		return nullptr;
	}

	GLuint textureHandle;
	glGenTextures(1, &textureHandle);

	glBindTexture(GL_TEXTURE_2D, textureHandle);

	glTexImage2D(GL_TEXTURE_2D, 0, glFormats[(int) format], width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(textureData);
		
	glBindTexture(GL_TEXTURE_2D, 0);

	Texture2D* result = new Texture2D();
	result->handle = textureHandle;
	result->dirty = false;

	result->wrapU = { GL_CLAMP_TO_EDGE, false };
	result->wrapV = { GL_CLAMP_TO_EDGE, false };
	result->minFilter = { GL_LINEAR_MIPMAP_LINEAR, false };
	result->magFilter = { GL_LINEAR, false };
	result->mipmapped = { false, false };

	return result;
}