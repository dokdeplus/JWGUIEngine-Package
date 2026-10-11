#pragma once
#include "Export.h"
#include "Image.h"
#include "ResourcePack.h"
#include "UIStrings.h"
#include <memory>
#include <string>
#include <vector>

#ifdef LoadImage
#undef LoadImage
#endif

namespace jwgui {

// Stack of packs. Last added is searched first (document overrides app).
class JWGUI_API ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    bool AddPackFile(const std::string& jwresPath, ResourcePackRole role = ResourcePackRole::Project);
    bool AddPackDirectory(const std::string& dir, ResourcePackRole role = ResourcePackRole::Project);
    // A pack inside a .jwpack (read-only). jwresEntry e.g. "res/app.jwres".
    bool AddPackArchive(std::shared_ptr<const ZipArchive> archive, const std::string& jwresEntry,
                        ResourcePackRole role = ResourcePackRole::Project);
    void AddPack(std::shared_ptr<ResourcePack> pack);
    void RemovePack(const ResourcePack* pack);
    void RemovePackByPath(const std::string& sourcePath);
    void Clear();

    bool Reload();
    ResourceRefreshResult Refresh();

    bool ImportFile(const std::string& sourcePath, const std::string& destDir, std::string* outId = nullptr);
    bool DeleteEntry(const std::string& id);
    bool DeleteFolder(const std::string& relativeDir);

    const std::vector<std::shared_ptr<ResourcePack>>& Packs() const { return m_packs; }
    ResourcePack* HostPack();
    const ResourcePack* HostPack() const;
    ResourcePack* ProjectPack();
    const ResourcePack* ProjectPack() const;

    const ResourceEntry* Find(const std::string& id) const;
    const ResourceEntry* Find(const std::string& id, ResourcePackRole role) const;
    const ResourceEntry* FindByFile(const std::string& path) const;
    bool Contains(const std::string& id) const { return Find(id) != nullptr; }
    bool Contains(const std::string& id, ResourcePackRole role) const { return Find(id, role) != nullptr; }
    std::string AbsolutePath(const std::string& id) const;

    bool ReadBytes(const std::string& id, std::vector<uint8_t>& out) const;
    bool ReadBytes(const std::string& id, std::vector<uint8_t>& out, ResourcePackRole role) const;
    const std::string* GetString(const std::string& id) const;
    Image LoadImage(const std::string& path);
    Image LoadImageById(const std::string& id);
    Image LoadImageById(const std::string& id, ResourcePackRole role);
    Image LoadImagePreview(const std::string& id);
    Image LoadImagePreview(const std::string& id, ResourcePackRole role);

    // Decoded no larger than maxEdge on the long edge (pickers, thumbnails).
    Image LoadImageScaled(const std::string& id, int maxEdge,
                          ResourcePackRole role = ResourcePackRole::Project);

    static constexpr int kPreviewMaxEdge = 2048;
    static constexpr int kThumbnailMaxEdge = 128; // picker tiles (refresh drops these too)

    bool SetEntryId(const std::string& currentId, const std::string& newId);
    bool SetEntryClass(const std::string& id, const std::string& className);

    std::vector<const ResourceEntry*> List() const;
    std::vector<const ResourceEntry*> List(ResourceType type) const;
    std::vector<const ResourceEntry*> List(ResourcePackRole role) const;
    std::vector<const ResourceEntry*> List(ResourceType type, ResourcePackRole role) const;
    std::vector<const ResourceEntry*> ListByClass(const std::string& className) const;
    std::vector<const ResourceEntry*> Search(const ResourceQuery& query) const;
    std::vector<const ResourceEntry*> Search(const ResourceQuery& query, ResourcePackRole role) const;
    std::vector<const ResourceEntry*> Search(const std::string& text) const;
    std::vector<const ResourceEntry*> Search(const std::string& text, ResourcePackRole role) const;

    // String tables of every pack (lang/<language>.json, lang/<language>/*.json),
    // lowest priority first: Host packs, then Project packs, each in the order
    // added. Translator notes (*.notes.json) are left out.
    std::vector<UIStrings::TableText> StringTables() const;
    // Goes up when packs are added, removed or refreshed (UIStrings reads the
    // tables again).
    unsigned int Generation() const { return m_generation; }

private:
    std::vector<std::shared_ptr<ResourcePack>> m_packs;
    unsigned int m_generation = 1;

    std::vector<const ResourceEntry*> Collect(const ResourceQuery& query) const;
    std::vector<const ResourceEntry*> Collect(const ResourceQuery& query, const ResourcePackRole* role) const;
    ResourcePack* PackFor(const std::string& id);
    const ResourcePack* PackFor(const std::string& id) const;
    ResourcePack* PackForFile(const std::string& relativeDir);
    ResourcePack* LastPack(ResourcePackRole role);
    const ResourcePack* LastPack(ResourcePackRole role) const;
    void InvalidateImages(const ResourceRefreshResult& diff);
    Image LoadImageFromPack(const std::string& id, ResourcePackRole role, int maxEdge = 0);
};

} // namespace jwgui
