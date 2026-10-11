#pragma once
#include "Export.h"
#include <string>
#include <vector>

namespace jwgui {

enum class ResourceType {
    Unknown = 0,
    Image,
    String,
    Font,
    Style,
    Binary
};

enum class ResourcePackRole {
    Host = 0,
    Project
};

JWGUI_API const char* ResourceTypeToString(ResourceType type);
JWGUI_API ResourceType ResourceTypeFromString(const char* text);
JWGUI_API ResourceType ResourceTypeFromExtension(const std::string& pathOrExt);

struct ResourceEntry {
    std::string id;
    std::string className;
    ResourceType type = ResourceType::Binary;
    std::string file;
    std::string value;
};

struct ResourceQuery {
    ResourceType type = ResourceType::Unknown;
    std::string id;
    std::string className;
    std::string text;
};

struct ResourceChange {
    std::string id;
    std::string file;
    ResourceType type = ResourceType::Binary;
};

struct ResourceRefreshResult {
    bool ok = true;
    std::vector<ResourceChange> added;
    std::vector<ResourceChange> removed;
    std::vector<ResourceChange> changed;
};

JWGUI_API bool ResourceHasClass(const ResourceEntry& entry, const std::string& className);
JWGUI_API bool ResourceMatches(const ResourceEntry& entry, const ResourceQuery& query);

} // namespace jwgui
