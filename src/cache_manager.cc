#include "cache_manager.h"

#include "config.h"
#include "indexer.h"
#include "language_server_api.h"
#include "platform.h"

#include <loguru/loguru.hpp>

#include <algorithm>
#include <unordered_map>

namespace {

// Manages loading caches from file paths for the indexer process.
struct RealCacheManager : ICacheManager {
  explicit RealCacheManager(Config* config) : config_(config) {}
  ~RealCacheManager() override = default;

  void WriteToCache(IndexFile& file) override {
    if (!config_->enableCacheWrite)
      return;

    std::string cache_path = GetCachePath(file.path);

    if (file.file_contents_.empty()) {
      LOG_S(ERROR) << "No cached file contents; performing potentially stale "
                   << "file-copy for " << file.path;
      CopyFileTo(cache_path, file.path);
    } else {
      WriteToFile(cache_path, file.file_contents_);
    }

    std::string indexed_content = Serialize(config_->cacheFormat, file);
    WriteToFile(AppendSerializationFormat(cache_path), indexed_content);
  }

  optional<std::string> LoadCachedFileContents(
      const std::string& path) override {
    if (!config_->enableCacheRead)
      return nullopt;

    return ReadContent(GetCachePath(path));
  }

  std::unique_ptr<IndexFile> RawCacheLoad(const std::string& path) override {
    if (!config_->enableCacheRead)
      return nullptr;

    std::string cache_path = GetCachePath(path);
    optional<std::string> file_content =
        ReadContent(AppendSerializationFormat(cache_path));
    if (!file_content)
      return nullptr;

    return Deserialize(config_->cacheFormat, path, *file_content,
                       IndexFile::kCurrentVersion);
  }

  std::string GetCachePath(const std::string& source_file) {
    assert(!config_->cacheDirectory.empty());
    std::string cache_file;
    size_t len = config_->projectRoot.size();
    if (StartsWith(source_file, config_->projectRoot)) {
      cache_file = EscapeFileName(config_->projectRoot) + '/' +
                   EscapeFileName(source_file.substr(len));
    } else {
      cache_file = EscapeFileName(source_file);
    }

    return config_->cacheDirectory + cache_file;
  }

  std::string AppendSerializationFormat(const std::string& base) {
    switch (config_->cacheFormat) {
      case SerializeFormat::Json:
        return base + ".json";
      case SerializeFormat::MessagePack:
        return base + ".mpack";
    }
    assert(false);
    return ".json";
  }

  Config* config_;
};

struct FakeCacheManager : ICacheManager {
  explicit FakeCacheManager(const std::vector<FakeCacheEntry>& entries)
      : entries_(entries) {}

  void WriteToCache(IndexFile& file) override { assert(false); }

  optional<std::string> LoadCachedFileContents(
      const std::string& path) override {
    for (const FakeCacheEntry& entry : entries_) {
      if (entry.path == path) {
        return entry.content;
      }
    }

    return nullopt;
  }

  std::unique_ptr<IndexFile> RawCacheLoad(const std::string& path) override {
    for (const FakeCacheEntry& entry : entries_) {
      if (entry.path == path) {
        return Deserialize(SerializeFormat::Json, path, entry.json, nullopt);
      }
    }

    return nullptr;
  }

  std::vector<FakeCacheEntry> entries_;
};

}  // namespace

// static
std::unique_ptr<ICacheManager> ICacheManager::Make(Config* config) {
  return MakeUnique<RealCacheManager>(config);
}

// static
std::unique_ptr<ICacheManager> ICacheManager::MakeFake(
    const std::vector<FakeCacheEntry>& entries) {
  return MakeUnique<FakeCacheManager>(entries);
}

ICacheManager::~ICacheManager() = default;

IndexFile* ICacheManager::TryLoad(const std::string& path) {
  auto it = caches_.find(path);
  if (it != caches_.end())
    return it->second.get();

  std::unique_ptr<IndexFile> cache = RawCacheLoad(path);
  if (!cache)
    return nullptr;

  caches_[path] = std::move(cache);
  return caches_[path].get();
}

std::unique_ptr<IndexFile> ICacheManager::TryTakeOrLoad(
    const std::string& path) {
  auto it = caches_.find(path);
  if (it != caches_.end()) {
    auto result = std::move(it->second);
    caches_.erase(it);
    return result;
  }

  return RawCacheLoad(path);
}

std::unique_ptr<IndexFile> ICacheManager::TakeOrLoad(const std::string& path) {
  auto result = TryTakeOrLoad(path);
  assert(result);
  return result;
}

void ICacheManager::IterateLoadedCaches(std::function<void(IndexFile*)> fn) {
  for (const auto& cache : caches_) {
    assert(cache.second);
    fn(cache.second.get());
  }
}
