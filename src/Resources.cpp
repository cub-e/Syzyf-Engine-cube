#include <Resources.h>

ResourceDatabase* const ResourceDatabase::Global = new ResourceDatabase();

void ResourceDatabase::Purge() {
	for (auto& resPair : this->loadedResources) {
		delete resPair.second.resource;
	}

	this->loadedResources.clear();
}