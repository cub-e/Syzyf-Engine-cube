#include <Resources.h>

ResourceDatabase* const ResourceDatabase::Global = new ResourceDatabase();

void ResourceDatabase::Free(const fs::path& path) {
	if (this->loadedResources.contains(path)) {
		delete this->loadedResources[path].resource;

		this->loadedResources[path].resource = nullptr;
	}
	else if (this->loadedGenericAssets.contains(path)) {
		delete this->loadedGenericAssets[path].resource;

		this->loadedGenericAssets[path].resource = nullptr;
	}
}

void ResourceDatabase::Purge() {
	for (auto& resPair : this->loadedResources) {
		delete resPair.second.resource;
	}

	this->loadedResources.clear();
}