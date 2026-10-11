#pragma once
#include "Export.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace jwgui {

// Plain ZIP with stored (uncompressed) entries and UTF-8 names. Used by the
// .jwpack app package and JWShareBoard's .jwsb board file: their contents
// (PNG, JPEG, PDF) are compressed already, and any zip tool can open them.

JWGUI_API uint32_t Crc32(const uint8_t* data, size_t size);

class JWGUI_API ZipWriter {
public:
    ZipWriter();

    // name uses '/' between folders, e.g. "res/icons/save.png".
    void Add(const std::string& name, const uint8_t* data, size_t size);
    void Add(const std::string& name, const std::string& text);
    // The whole archive. The writer is empty afterwards.
    std::vector<uint8_t> Finish();

private:
    struct Central {
        std::string name;
        uint32_t crc = 0;
        uint32_t size = 0;
        uint32_t offset = 0;
    };
    std::vector<uint8_t> m_out;
    std::vector<Central> m_central;
    uint32_t m_dosTime = 0;
    uint32_t m_dosDate = 0;
};

// A file inside an archive. data points into the archive bytes.
struct ZipEntryView {
    std::string name;
    const uint8_t* data = nullptr;
    size_t size = 0;
};

enum class ZipReadError {
    None,
    TooSmall,     // shorter than an end record: not a zip
    NoDirectory,  // no end record: not a zip, or cut off
    Corrupt,      // a header points outside the file or has a wrong signature
    Compressed,   // an entry is compressed (re-packed by another zip tool)
    Truncated,    // an entry's bytes run past the end
    BadCrc        // an entry's bytes do not match its checksum
};

// Lists the files (folders are skipped). On failure badEntry (optional)
// receives the name of the entry that failed, when there is one.
JWGUI_API bool ReadZip(const uint8_t* data, size_t size, std::vector<ZipEntryView>& entries,
                       ZipReadError& error, std::string* badEntry = nullptr);

// Owns the bytes of a read archive and finds entries by name.
class JWGUI_API ZipArchive {
public:
    bool Open(std::vector<uint8_t> bytes, ZipReadError* error = nullptr);
    bool OpenFile(const std::string& path, ZipReadError* error = nullptr);

    const ZipEntryView* Find(const std::string& name) const;
    const std::vector<ZipEntryView>& Entries() const { return m_entries; }
    bool Read(const std::string& name, std::vector<uint8_t>& out) const;
    bool ReadText(const std::string& name, std::string& out) const;

private:
    std::vector<uint8_t> m_bytes;
    std::vector<ZipEntryView> m_entries;
    std::unordered_map<std::string, size_t> m_byName;
};

// Reads a whole file (UTF-8 path) into memory.
JWGUI_API bool ReadFileBytes(const std::string& path, std::vector<uint8_t>& out);

} // namespace jwgui
