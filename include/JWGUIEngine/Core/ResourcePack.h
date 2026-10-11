#pragma once
#include "Export.h"
#include "ResourceTypes.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace jwgui {

class ZipArchive;
struct ZipEntryView;

// One .jwres policy file plus its directory. Folder rules are scanned on Load;
// per-file id/class are optional. Save writes the policy, not the expanded list.
class JWGUI_API ResourcePack {
public:
    ResourcePack() = default;

    bool Load(const std::string& jwresPath);
    bool LoadFromString(const std::string& xml, const std::string& rootDir);
    bool LoadDirectory(const std::string& dir);
    // Files live in an archive (a .jwpack) instead of a folder. jwresEntry is
    // the policy file inside it ("res/app.jwres"); file paths are relative to
    // its folder, and Folder rules scan the archive's names. Read-only: Import,
    // Delete and Save fail, AbsolutePath is empty.
    bool LoadFromArchive(std::shared_ptr<const ZipArchive> archive, const std::string& jwresEntry);
    bool IsArchive() const { return m_archive != nullptr; }
    bool Reload();
    ResourceRefreshResult Refresh();
    bool Save() const;
    bool Save(const std::string& jwresPath) const;

    bool ImportFile(const std::string& sourcePath, const std::string& destDir, std::string* outId = nullptr);
    bool DeleteEntry(const std::string& id);
    bool DeleteFolder(const std::string& relativeDir);

    const std::string& Root() const { return m_root; }
    const std::string& SourcePath() const { return m_sourcePath; }
    ResourcePackRole Role() const { return m_role; }
    void SetRole(ResourcePackRole role) { m_role = role; }

    const ResourceEntry* Find(const std::string& id) const;
    const ResourceEntry* FindByFile(const std::string& path) const;
    bool Contains(const std::string& id) const { return Find(id) != nullptr; }
    std::string AbsolutePath(const std::string& id) const;

    bool ReadBytes(const std::string& id, std::vector<uint8_t>& out) const;
    const std::string* GetString(const std::string& id) const;

    std::vector<const ResourceEntry*> List() const;
    std::vector<const ResourceEntry*> List(ResourceType type) const;
    std::vector<const ResourceEntry*> ListByClass(const std::string& className) const;
    std::vector<const ResourceEntry*> Search(const ResourceQuery& query) const;
    std::vector<const ResourceEntry*> Search(const std::string& text) const;

    bool SetEntryId(const std::string& currentId, const std::string& newId);
    bool SetEntryClass(const std::string& id, const std::string& className);

private:
    struct Policy {
        enum class Kind { Folder, File, String };
        Kind kind = Kind::File;
        ResourceType type = ResourceType::Binary;
        std::string id;
        std::string className;
        std::string file;
        std::string path;
        std::string pattern;
        std::string prefix;
        std::string value;
        bool recursive = true;
    };

    std::string m_root;
    std::string m_sourcePath;
    ResourcePackRole m_role = ResourcePackRole::Project;
    std::vector<Policy> m_policies;
    std::vector<ResourceEntry> m_entries;
    std::unordered_map<std::string, size_t> m_byId;
    std::unordered_map<std::string, uint64_t> m_stamps;
    std::shared_ptr<const ZipArchive> m_archive;
    std::string m_archivePrefix; // "res/": folder of the .jwres inside the archive

    void ClearIndex();
    bool ParseDocument(const void* xmlDoc);
    bool ApplyPolicies();
    bool ApplyFolder(const Policy& policy);
    bool AddOrPatch(ResourceEntry entry);
    void UpsertFileOverride(const ResourceEntry& entry, const std::string& previousId);
    void RemovePolicyForId(const std::string& id);
    bool PolicyCoversFile(const std::string& relativeFile) const;
    bool WritePolicy() const;
    void RebuildStamps();
    ResourceRefreshResult DiffAgainst(const std::vector<ResourceEntry>& previous,
                                      const std::unordered_map<std::string, uint64_t>& previousStamps) const;
    std::string MakeId(const std::string& relativePath, const std::string& prefix) const;
    std::string AbsoluteFile(const std::string& relative) const;
    bool HasFile(const std::string& relative) const;
    const ZipEntryView* ArchiveEntry(const std::string& relative) const;
};

} // namespace jwgui
