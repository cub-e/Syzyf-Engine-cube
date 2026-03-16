#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <concepts>
#include <typeindex>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

class Resource {
public:
  virtual ~Resource() = default;
};

template<class T, typename... T_Params>
concept Loadable = requires(fs::path p, T_Params... loadParams) {
  { T::Load(p, loadParams...) } -> std::convertible_to<T>;
} && std::derived_from<T, Resource>;

namespace internal {
  struct ResourceHandle {
    std::size_t index = 0;
    std::size_t generation = 0;

    bool IsValid() const { return generation != 0; }
  };
}

class ResourceDatabase;

template <typename T>
class ResourceRef {
friend class ResourceDatabase;
private:
  internal::ResourceHandle handle;

  explicit ResourceRef(internal::ResourceHandle h);

public:
  ResourceRef();
  ResourceRef(const ResourceRef& other);
  ResourceRef& operator=(const ResourceRef& other);
  ResourceRef(ResourceRef&& other) noexcept;
  ResourceRef& operator=(ResourceRef&& other) noexcept;
  ~ResourceRef();

  T* Get() const;
  T* operator->() const;
  bool IsValid() const;
};

class IResourcePool {
public:
  virtual ~IResourcePool() = default;
  virtual void FreeUnreferenced() = 0;
  virtual void FreeAll() = 0;
};

template <typename T>
class ResourcePool : public IResourcePool {
friend class ResourceDatabase;
private:
  std::vector<std::optional<T>> resources;
  std::vector<std::size_t> generations;
  std::vector<uint32_t> referenceCounts;
  std::vector<std::size_t> freeList;

  std::unordered_map<fs::path, internal::ResourceHandle> pathToHandle;
  std::unordered_map<std::size_t, fs::path> indexToPath;

  internal::ResourceHandle Allocate(const fs::path& path, T&& resource) {
    std::size_t index;
    if (freeList.empty()) {
      index = resources.size();
      this->resources.push_back(std::move(resource));
      this->generations.push_back(1);
      this->referenceCounts.push_back(1);
    } else {
      index = this->freeList.back();
      this->freeList.pop_back();
      this->resources[index].emplace(std::move(resource));
      this->generations[index]++;
      this->referenceCounts[index] = 1;
    }

    internal::ResourceHandle handle { index, this->generations[index] };
    this->pathToHandle[path] = handle;
    this->indexToPath[index] = path;

    return handle;
  }

  void FreeSlot(std::size_t index) {
    this->resources[index].reset();

    this->generations[index]++;

    fs::path path = indexToPath[index];
    this->pathToHandle.erase(path);
    this->indexToPath.erase(index);

    this->freeList.push_back(index);
  }

  void AddReference(internal::ResourceHandle handle) {
    if (Resolve(handle) != nullptr) {
      this->referenceCounts[handle.index]++;
    }
  }

  void Release(internal::ResourceHandle handle) {
    if (Resolve(handle) != nullptr && this->referenceCounts[handle.index] > 0) {
      this->referenceCounts[handle.index]--;
    }
  }

public:
  ~ResourcePool() override {
    FreeAll();
  }

  template<typename... T_Params>
  internal::ResourceHandle Get(const fs::path& path, T_Params... loadParams) {
    auto handle = this->pathToHandle.find(path);
    if (handle != this->pathToHandle.end()) {
      referenceCounts[handle->second.index]++;
      return handle->second;
    }

    return Allocate(path, T::Load(path, loadParams...));
  }

  T* Resolve(internal::ResourceHandle handle) {
    if (!handle.IsValid() || handle.index >= generations.size()) {
      return nullptr;
    }
    if (this->generations[handle.index] != handle.generation || !this->resources[handle.index].has_value()) {
      return nullptr;
    }

    return &this->resources[handle.index].value();
  }

  void FreeUnreferenced() override {
    for (std::size_t i = 0; i < this->resources.size(); ++i) {
      if (this->resources[i].has_value() && this->referenceCounts[i] == 0) {
        FreeSlot(i);
      }
    }
  }

  void FreeAll() override {
    for (std::size_t i = 0; i < this->resources.size(); ++i) {
      if (this->resources[i].has_value()) {
        FreeSlot(i);
      }
    }
  }
};

class ResourceDatabase {
private:
  template <typename T> friend class ResourceRef;

  std::unordered_map<std::type_index, std::unique_ptr<IResourcePool>> pools;

  template <typename T>
  ResourcePool<T>* GetOrCreatePool() {
    std::type_index typeIndex = std::type_index(typeid(T));
    if (!this->pools.contains(typeIndex)) {
      this->pools[typeIndex] = std::make_unique<ResourcePool<T>>();
    }
    return static_cast<ResourcePool<T>*>(this->pools[typeIndex].get());
  }

  template <typename T_Resource>
  T_Resource* Resolve(internal::ResourceHandle handle) {
    return GetOrCreatePool<T_Resource>()->Resolve(handle);
  }

  template <typename T_Resource>
  void AddReference(internal::ResourceHandle handle) {
    GetOrCreatePool<T_Resource>()->AddReference(handle);
  }

  template <typename T_Resource>
  void Release(internal::ResourceHandle handle) {
    GetOrCreatePool<T_Resource>()->Release(handle);
  }

public:
  inline static ResourceDatabase* Global = nullptr;

  template<class T_Resource, typename... T_Params>
    requires(Loadable<T_Resource, T_Params...>)
  ResourceRef<T_Resource> Get(const fs::path& resourcePath, T_Params... loadParams) {
    return ResourceRef<T_Resource>(GetOrCreatePool<T_Resource>()->Get(resourcePath, loadParams...));
  }

  void FreeUnreferenced() {
    for (auto& [type, pool] : pools) {
      pool->FreeUnreferenced();
    }
  }

  void FreeAll() {
    for (auto& [type, pool] : pools) {
      pool->FreeAll();
    }
    pools.clear();
  }
};

template <typename T>
ResourceRef<T>::ResourceRef() : handle { 0, 0 } {}

template <typename T>
ResourceRef<T>::ResourceRef(internal::ResourceHandle h) : handle(h) {}

template <typename T>
ResourceRef<T>::ResourceRef(const ResourceRef& other) : handle(other.handle) {
  if (handle.IsValid()) {
    ResourceDatabase::Global->AddReference<T>(handle);
  }
}

template <typename T>
ResourceRef<T>& ResourceRef<T>::operator=(const ResourceRef& other) {
  if (this != &other) {
    if (handle.IsValid()) {
      ResourceDatabase::Global->Release<T>(handle);
    }
    handle = other.handle;
    if (handle.IsValid()) {
      ResourceDatabase::Global->AddReference<T>(handle);
    }
  }
  return *this;
}

template <typename T>
ResourceRef<T>::ResourceRef(ResourceRef&& other) noexcept : handle(other.handle) {
  other.handle = { 0, 0 };
}

template <typename T>
ResourceRef<T>& ResourceRef<T>::operator=(ResourceRef&& other) noexcept {
  if (this != &other) {
    if (handle.IsValid()) {
      ResourceDatabase::Global->Release<T>(handle);
    }
    handle = other.handle;
    other.handle = { 0, 0 };
  }
  return *this;
}

template <typename T>
ResourceRef<T>::~ResourceRef() {
  if (handle.IsValid()) {
    ResourceDatabase::Global->Release<T>(handle);
  }
}

template <typename T>
T* ResourceRef<T>::Get() const {
  return ResourceDatabase::Global->Resolve<T>(handle);
}

template <typename T>
T* ResourceRef<T>::operator->() const {
  return Get();
}

template <typename T>
bool ResourceRef<T>::IsValid() const {
  return handle.IsValid() && Get() != nullptr;
}
