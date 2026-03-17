#include <Shader.h>

#include <fstream>
#include <sstream>
#include <queue>
#include <malloc.h>

#include <PreComp.h>
#include <Material.h>

#include <spdlog/spdlog.h>

struct ShaderCodeSegment {
	char* str;
	size_t length;
};

struct ShaderFile {
	fs::path filePath;
	char* content;
};

struct ShaderCode {
	std::vector<ShaderFile> loadedFiles;
	std::vector<ShaderCodeSegment> segments;
};

ShaderVariantInfo::ShaderVariantInfo(std::initializer_list<ShaderVariantPoint> variantPoints):
variantPoints(variantPoints) { }

ShaderBase::ShaderBase(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle):
filePath(filePath),
variantInfo(variantInfo),
handle(handle) { }

ShaderBase::~ShaderBase() {
  if (this->handle == 0) return;
	glDeleteShader(this->handle);
}

ShaderBase::ShaderBase(ShaderBase&& other) noexcept :
  filePath(std::move(other.filePath)),
  variantInfo(std::move(other.variantInfo)),
  handle(other.handle) {
    other.handle = 0;
}

ShaderBase& ShaderBase::operator=(ShaderBase&& other) noexcept {
  if (this != &other) {
    if (this->handle != 0) {
      glDeleteShader(this->handle);
    }
    filePath = std::move(other.filePath);
    variantInfo = std::move(other.variantInfo);
    handle = other.handle;

    other.handle = 0;
  }

  return *this;
}

char* LoadFile(const fs::path& filePath) {
	fs::directory_entry shaderFile(filePath);

	// if (!shaderFile.exists()) {
	// 	throw shader::shader_missing_file_exception(shaderFile);
	// }

	char* buf = new char[shaderFile.file_size() + 1];
	
	std::ifstream shaderFileStream(filePath, std::ios::binary);

	shaderFileStream.read(buf, shaderFile.file_size());
	
	buf[shaderFile.file_size()] = '\0';
	
	return buf;
}

char* LoadFile(const fs::path& filePath, GLenum& shaderType) {
	fs::directory_entry shaderFile(filePath);

	shaderType = GL_INVALID_ENUM;
	
	if (filePath.extension() == ".vert") {
		shaderType = GL_VERTEX_SHADER;
	}
	else if (filePath.extension() == ".frag") {
		shaderType = GL_FRAGMENT_SHADER;
	}
	else if (filePath.extension() == ".geom") {
		shaderType = GL_GEOMETRY_SHADER;
	}
	else if (filePath.extension() == ".tess_eval") {
		shaderType = GL_TESS_EVALUATION_SHADER;
	}
	else if (filePath.extension() == ".tess_ctrl") {
		shaderType = GL_TESS_CONTROL_SHADER;
	}
	else if (filePath.extension() == ".comp") {
		shaderType = GL_COMPUTE_SHADER;
	}
	else {
		// throw shader::shader_unknown_type_exception(path_to_file);
	}

	return LoadFile(filePath);
}

int VersionPresent(const std::string& shaderCode) {
	std::smatch versionMatch;

	if (std::regex_search(shaderCode.begin(), shaderCode.end(), versionMatch, Regex::shaderHeaderRegex)) {
		return versionMatch.position() + versionMatch.length();
	}

	return 0;
}

VertexSpec GetVertexSpec(const std::string_view& shaderCode) {
	auto inputSearch = std::regex_iterator(shaderCode.cbegin(), shaderCode.cend(), Regex::shaderInputRegex);
	decltype(inputSearch) shaderCodeEnd;
	std::vector<VertexInput> spec;

	for (auto i = inputSearch; i != shaderCodeEnd; ++i) {
		const auto match = *i;

		const std::string semanticString = match[2].str();

		const VertexInputType semantic = VertexSpec::TypeFromSemantic(semanticString);

		if ((int) semantic < 0) {
			spdlog::error("Unknown shader input semantic: " + semanticString);
		}

		const std::string typeStr = match[3].str();
		int length = 0;

		length = typeStr.back() - '0';

		if (length < 0) {
			length = 1;
		}

		spec.push_back(VertexInput(semantic, length));
	}

	return VertexSpec(spec);
}

GLuint CompileShader(fs::path filePath, GLenum shaderType, VertexSpec* spec = nullptr) {
	GLuint shaderHandle = glCreateShader(shaderType);
	ShaderCode code;

	std::queue<fs::path> filesToLoad;
	filesToLoad.push(filePath);

	while (!filesToLoad.empty()) {
		fs::path loadedFilePath = filesToLoad.front();
		filesToLoad.pop();

		if (std::any_of(code.loadedFiles.begin(), code.loadedFiles.end(), [loadedFilePath](const ShaderFile& f) -> bool {
			return f.filePath == loadedFilePath;
		} )) {
			continue;
		}

		ShaderFile loadedFile;
		loadedFile.filePath = loadedFilePath;
		loadedFile.content = LoadFile(loadedFilePath);

		code.loadedFiles.push_back(loadedFile);

		std::string_view codeView(loadedFile.content, strlen(loadedFile.content));
		auto codeIt = std::regex_iterator(codeView.cbegin(), codeView.cend(), Regex::shaderIncludeRegex);

		for (decltype(codeIt) last; codeIt != last; ++codeIt) {
			fs::path includedFile = BaseShaderPath / (*codeIt)[1].str();
			filesToLoad.push(includedFile);
		}
	}

	code.segments.push_back({code.loadedFiles[0].content, strlen(code.loadedFiles[0].content)});
	std::vector<fs::path> expandedFiles;
	expandedFiles.push_back(filePath);

	bool trip = false;
	while (expandedFiles.size() != code.loadedFiles.size()) {
		std::vector<ShaderCodeSegment> newCodeSegments;

		for (const auto& segment : code.segments) {
			size_t pointer = 0;
			std::string_view codeView(segment.str, segment.length);
			auto codeIt = std::regex_iterator(codeView.cbegin(), codeView.cend(), Regex::shaderIncludeRegex);

			for (decltype(codeIt) last; codeIt != last; ++codeIt) {
				const auto& match = *codeIt;

				newCodeSegments.push_back({segment.str + pointer, match.position() - pointer});
				
				pointer = match.position() + match.length();

				fs::path includedFilePath = BaseShaderPath / match[1].str();

				if (!std::any_of(expandedFiles.begin(), expandedFiles.end(), [includedFilePath](const fs::path& p) -> bool {
					return p == includedFilePath;
				} )) {
					const auto& includedFile = *std::find_if(code.loadedFiles.begin(), code.loadedFiles.end(), [includedFilePath](const ShaderFile& f) -> bool {
						return f.filePath == includedFilePath;
					} );

					newCodeSegments.push_back({includedFile.content, strlen(includedFile.content)});
					expandedFiles.push_back(includedFilePath);
				}
			}

			newCodeSegments.push_back({segment.str + pointer, segment.length - pointer});
		}

		code.segments = newCodeSegments;
	}

	char** segmentsStrings = (char**) alloca(sizeof(char*) * code.segments.size());
	int* segmentsLengths = (int*) alloca(sizeof(int) * code.segments.size());

	for (int i = 0; i < code.segments.size(); i++) {
		segmentsStrings[i] = code.segments[i].str;
		segmentsLengths[i] = code.segments[i].length;
	}

	glShaderSource(shaderHandle, code.segments.size(), segmentsStrings, segmentsLengths);

	glCompileShader(shaderHandle);

	int compileSuccess;
	
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

	if (!compileSuccess) {
		int logLength = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLength);
		char* compileMsg = (char*) alloca(sizeof(char) * logLength);
		glGetShaderInfoLog(shaderHandle, logLength, nullptr, compileMsg);

		spdlog::error("Error compiling shader {}:\n{}", filePath.string(), std::string(compileMsg));

		int sourceLength = 0;
		glGetShaderiv(shaderHandle, GL_SHADER_SOURCE_LENGTH, &sourceLength);
		char* shaderSource = (char*)alloca(sizeof(char) * sourceLength);

		glGetShaderSource(shaderHandle, sourceLength, nullptr, shaderSource);

		std::istringstream inss(shaderSource);
		std::stringstream outss;

		int lineNum = 1;
		for (std::string line; std::getline(inss, line); ) {
			outss << std::setw(3) << lineNum++ << "| " << line << "\n";
		}

		spdlog::error("Shader source: \n{}", outss.str());

    glDeleteShader(shaderHandle);
    shaderHandle = 0;

		// throw shader::shader_compilation_exception(path_to_file, compile_msg);
	} else {
    spdlog::info("Compiled shader {}", filePath.filename().string());
    if (spec != nullptr) {
      *spec = GetVertexSpec(code.loadedFiles[0].content);
    }
  }

	for (auto& file : code.loadedFiles) {
		delete[] file.content;
	}

	return shaderHandle;
}

const fs::path& ShaderBase::GetFilePath() const {
	return this->filePath;
}

std::string ShaderBase::GetName() const {
	return this->filePath.stem().string();
}
const ShaderVariantInfo& ShaderBase::GetVariantInfo() const {
	return this->variantInfo;
}
GLuint ShaderBase::GetHandle() const {
	return this->handle;
}

VertexShader::VertexShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle, VertexSpec spec):
ShaderBase(filePath, variantInfo, handle),
vertexSpec(spec) { }

VertexShader::VertexShader(VertexShader&& other) noexcept :
  ShaderBase(std::move(other)), vertexSpec(std::move(other.vertexSpec)) {}

VertexShader& VertexShader::operator=(VertexShader&& other) noexcept {
  ShaderBase::operator=(std::move(other));
  vertexSpec = std::move(other.vertexSpec);
  return *this;
}

VertexShader VertexShader::Load(fs::path filePath) {
  VertexSpec spec { std::vector<VertexInput>{} };
  GLuint handle = CompileShader(filePath, GL_VERTEX_SHADER, &spec);

	return VertexShader(filePath, {}, handle, spec);
}

const VertexSpec& VertexShader::GetVertexSpec() const {
	return this->vertexSpec;
}

GLenum VertexShader::GetType() const {
	return GL_VERTEX_SHADER;
}

GeometryShader::GeometryShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle):
ShaderBase(filePath, variantInfo, handle) { }

GeometryShader::GeometryShader(GeometryShader&& other) noexcept :
  ShaderBase(std::move(other)) {}

GeometryShader& GeometryShader::operator=(GeometryShader&& other) noexcept {
  ShaderBase::operator=(std::move(other));
  return *this;
}

GeometryShader GeometryShader::Load(fs::path filePath) {
  GLuint handle = CompileShader(filePath, GL_GEOMETRY_SHADER);

	return GeometryShader(filePath, {}, handle);
}

GLenum GeometryShader::GetType() const {
	return GL_GEOMETRY_SHADER;
}

TesselationEvaluationShader::TesselationEvaluationShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle):
ShaderBase(filePath, variantInfo, handle) { }

TesselationEvaluationShader::TesselationEvaluationShader(TesselationEvaluationShader&& other) noexcept :
  ShaderBase(std::move(other)) {}

TesselationEvaluationShader& TesselationEvaluationShader::operator=(TesselationEvaluationShader&& other) noexcept {
  ShaderBase::operator=(std::move(other));
  return *this;
}

TesselationEvaluationShader TesselationEvaluationShader::Load(fs::path filePath) {
  GLuint handle = CompileShader(filePath, GL_TESS_EVALUATION_SHADER);
  return TesselationEvaluationShader(filePath, {}, handle);
}

GLenum TesselationEvaluationShader::GetType() const {
	return GL_TESS_EVALUATION_SHADER;
}

TesselationControlShader::TesselationControlShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle):
ShaderBase(filePath, variantInfo, handle) { }

TesselationControlShader::TesselationControlShader(TesselationControlShader&& other) noexcept :
  ShaderBase(std::move(other)) {}

TesselationControlShader& TesselationControlShader::operator=(TesselationControlShader&& other) noexcept {
  ShaderBase::operator=(std::move(other));
  return *this;
}

TesselationControlShader TesselationControlShader::Load(fs::path filePath) {
  GLuint handle = CompileShader(filePath, GL_TESS_CONTROL_SHADER);
  return TesselationControlShader(filePath, {}, handle);
}

GLenum TesselationControlShader::GetType() const {
	return GL_TESS_CONTROL_SHADER;
}

PixelShader::PixelShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle):
ShaderBase(filePath, variantInfo, handle) { }

PixelShader::PixelShader(PixelShader&& other) noexcept :
  ShaderBase(std::move(other)) {}

PixelShader& PixelShader::operator=(PixelShader&& other) noexcept {
  ShaderBase::operator=(std::move(other));
  return *this;
}

PixelShader PixelShader::Load(fs::path filePath) {
  GLuint handle = CompileShader(filePath, GL_FRAGMENT_SHADER);
  return PixelShader(filePath, {}, handle);
}

GLenum PixelShader::GetType() const {
	return GL_FRAGMENT_SHADER;
}

ComputeShader::ComputeShader(fs::path filePath, ShaderVariantInfo variantInfo, GLuint handle):
ShaderBase(filePath, variantInfo, handle) { }

ComputeShader::ComputeShader(ComputeShader&& other) noexcept :
  ShaderBase(std::move(other)) {}

ComputeShader& ComputeShader::operator=(ComputeShader&& other) noexcept {
  ShaderBase::operator=(std::move(other));
  return *this;
}

ComputeShader ComputeShader::Load(fs::path filePath) {
  GLuint handle = CompileShader(filePath, GL_COMPUTE_SHADER);
  return ComputeShader(filePath, {}, handle);
}

GLenum ComputeShader::GetType() const {
	return GL_COMPUTE_SHADER;
}

ShaderBuilder& ShaderBuilder::WithVertexShader(ResourceRef<VertexShader> vertexShader) {
	this->vertexShader = vertexShader;

	return *this;
}

ShaderBuilder& ShaderBuilder::WithGeometryShader(ResourceRef<GeometryShader> geometryShader) {
	this->geometryShader = geometryShader;

	return *this;
}

ShaderBuilder& ShaderBuilder::WithTessEvaluationShader(ResourceRef<TesselationEvaluationShader> tessEvalShader) {
	this->tessEvalShader = tessEvalShader;

	return *this;
}
ShaderBuilder& ShaderBuilder::WithTessControlShader(ResourceRef<TesselationControlShader> tessCtrlShader) {
	this->tessCtrlShader = tessCtrlShader;

	return *this;
}

ShaderBuilder& ShaderBuilder::WithPixelShader(ResourceRef<PixelShader> pixelShader) {
	this->pixelShader = pixelShader;

	return *this;
}

std::shared_ptr<ShaderProgram> ShaderBuilder::Link() {
  GLuint programHandle = glCreateProgram();

  if (this->vertexShader.IsValid()) glAttachShader(programHandle, this->vertexShader->GetHandle());
  if (this->pixelShader.IsValid()) glAttachShader(programHandle, this->pixelShader->GetHandle());
  if (this->geometryShader.IsValid()) glAttachShader(programHandle, this->geometryShader->GetHandle());

  if (this->tessCtrlShader.IsValid() && this->tessEvalShader.IsValid()) {
    glAttachShader(programHandle, this->tessEvalShader->GetHandle());
    glAttachShader(programHandle, this->tessCtrlShader->GetHandle());
  }

  glLinkProgram(programHandle);

	int compileSuccess;
	char compileMsg[512];

	glGetProgramiv(programHandle, GL_LINK_STATUS, &compileSuccess);
	if (!compileSuccess) {
		glGetProgramInfoLog(programHandle, 512, NULL, compileMsg);

		spdlog::error("Error linking shader:\n{}", compileMsg);
	}

  std::shared_ptr<ShaderProgram> prog(new ShaderProgram(
		this->vertexShader,
		this->geometryShader,
		this->pixelShader,
		programHandle
	));

	if (this->tessCtrlShader.IsValid() && this->tessEvalShader.IsValid()) {
		prog->flags = ShaderProgramFlags::UsePatches;
	}
	else {
		prog->flags = ShaderProgramFlags::None;
	}

	return prog; 
}

ShaderProgram::ShaderProgram(ResourceRef<VertexShader> vertexShader, ResourceRef<GeometryShader> geometryShader, ResourceRef<PixelShader> pixelShader, GLuint handle):
vertexShader(vertexShader),
geometryShader(geometryShader),
pixelShader(pixelShader),
handle(handle) {
	this->uniforms = UniformSpec(this);
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(this->handle);
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept :
  vertexShader(std::move(other.vertexShader)),
  geometryShader(std::move(other.geometryShader)),
  pixelShader(std::move(other.pixelShader)),
  uniforms(std::move(other.uniforms)),
  flags(other.flags),
  handle(other.handle) {
  other.handle = 0;
  this->uniforms = UniformSpec(this);
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
  if (this != &other) {
    if (this->handle != 0) {
      glDeleteProgram(this->handle);
    }

    this->vertexShader = std::move(other.vertexShader);
    this->geometryShader = std::move(other.geometryShader);
    this->pixelShader = std::move(other.pixelShader);
    this->uniforms = std::move(other.uniforms);
    this->flags = other.flags;
    this->handle = other.handle;

    other.handle = 0;
    this->uniforms = UniformSpec(this);
  }
  return *this;
}

ShaderBuilder ShaderProgram::Build() {
	return ShaderBuilder{};
}

void ShaderProgram::Rebuild() {
  glDeleteProgram(this->handle);

  GLuint programHandle = glCreateProgram();

  if (this->vertexShader.IsValid()) {
    this->vertexShader = Resources::Global->Get<VertexShader>(this->vertexShader->GetFilePath());
    glAttachShader(programHandle, this->vertexShader->GetHandle());
  }
  if (this->pixelShader.IsValid()) {
    this->pixelShader = Resources::Global->Get<PixelShader>(this->pixelShader->GetFilePath());
    glAttachShader(programHandle, this->pixelShader->GetHandle());
  } 
  if (this->geometryShader.IsValid()) {
    this->geometryShader = Resources::Global->Get<GeometryShader>(this->geometryShader->GetFilePath());
    glAttachShader(programHandle, this->geometryShader->GetHandle());
  } 

  glLinkProgram(programHandle);

	int compileSuccess;
	char compileMsg[512];

	glGetProgramiv(programHandle, GL_LINK_STATUS, &compileSuccess);
	if (!compileSuccess) {
		glGetProgramInfoLog(programHandle, 512, NULL, compileMsg);

		spdlog::error("Error linking shader:\n{}", compileMsg);
	}

	this->handle = programHandle;
  this->uniforms = UniformSpec(this);
}

GLuint ShaderProgram::GetHandle() const {
	return this->handle;
}

const UniformSpec& ShaderProgram::GetUniforms() const {
	return this->uniforms;
}
const VertexSpec& ShaderProgram::GetVertexSpec() const {
	return this->vertexShader->GetVertexSpec();
}

bool ShaderProgram::IgnoresDepthPrepass() const {
	return ((unsigned int) this->flags & (unsigned int) ShaderProgramFlags::IgnoreDepthPrepass) != 0;
}

bool ShaderProgram::CastsShadows() const {
	return ((unsigned int) this->flags & (unsigned int) ShaderProgramFlags::DontCastShadows) == 0;
}

bool ShaderProgram::UsesPatches() const {
	return ((unsigned int) this->flags & (unsigned int) ShaderProgramFlags::UsePatches) != 0;
}

void ShaderProgram::SetIgnoresDepthPrepass(bool ignores) {
	unsigned int temp = (unsigned int) ShaderProgramFlags::IgnoreDepthPrepass;
	temp = ~temp;

	temp = (unsigned int) this->flags & temp;

	temp |= (unsigned int) ShaderProgramFlags::IgnoreDepthPrepass * ignores;

	this->flags = (ShaderProgramFlags) temp;
}

void ShaderProgram::SetCastsShadows(bool casts) {
	unsigned int temp = (unsigned int) ShaderProgramFlags::DontCastShadows;
	temp = ~temp;

	temp = (unsigned int) this->flags & temp;

	temp |= (unsigned int) ShaderProgramFlags::DontCastShadows * !casts;

	this->flags = (ShaderProgramFlags) temp;
}

ComputeShaderProgram::ComputeShaderProgram(ResourceRef<ComputeShader> computeShader) {
	assert(computeShader);

	this->handle = glCreateProgram();
	glAttachShader(this->handle, computeShader->GetHandle());

	glLinkProgram(this->handle);

	int compileSuccess;
	char compileMsg[512];

	glGetProgramiv(this->handle, GL_LINK_STATUS, &compileSuccess);
	if (!compileSuccess) {
		glGetProgramInfoLog(this->handle, 512, NULL, compileMsg);

		spdlog::error("Error linking compute shader program:\n{}", compileMsg);
	}

	this->uniforms = UniformSpec(this);	
}

ComputeShaderProgram::~ComputeShaderProgram() {
	glDeleteProgram(this->handle);
}

GLuint ComputeShaderProgram::GetHandle() const {
	return this->handle;
}

const UniformSpec& ComputeShaderProgram::GetUniforms() const {
	return this->uniforms;
}

ComputeShaderDispatch::ComputeShaderDispatch(ResourceRef<ComputeShader> compShader):
ComputeShaderDispatch(std::make_shared<ComputeShaderProgram>(compShader)) { }

ComputeShaderDispatch::ComputeShaderDispatch(std::shared_ptr<ComputeShaderProgram> program) {
	this->program = program;
	this->dispatchData = std::make_unique<ComputeDispatchData>(program);
}

void ComputeShaderDispatch::Dispatch(int groupsX, int groupsY, int groupsZ) const {
	this->dispatchData->Bind();

	glDispatchCompute(groupsX, groupsY, groupsZ);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

ComputeDispatchData* ComputeShaderDispatch::GetData() {
	return this->dispatchData.get();
}
ComputeShaderProgram* ComputeShaderDispatch::GetProgram() {
	return this->program.get();
}
