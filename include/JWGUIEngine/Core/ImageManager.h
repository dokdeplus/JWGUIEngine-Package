#pragma once
#include "Export.h"
#include "Image.h"
#include <cstddef>
#include <string>
#include <unordered_map>

namespace jwgui {

// Named image cache owned by Application. IRenderer Load/Free stay public for
// callers that want to manage handles themselves.
class JWGUI_API ImageManager {
public:
    ImageManager() = default;
    ~ImageManager();

    Image Get(const std::string& key) const;
    bool Contains(const std::string& key) const;

    // Named: kept until Unregister/Clear (other Image copies may still live).
    Image LoadFile(const std::string& key, const std::string& path);
    Image LoadMemory(const std::string& key, const void* data, size_t size, int maxEdge = 0);
    Image Register(const std::string& key, IRenderer::ImageHandle handle);

    void Unregister(const std::string& key);
    void Clear();

    // Anonymous: not stored. Last Image copy frees the handle.
    Image CreateFromFile(const std::string& path);
    Image CreateFromMemory(const void* data, size_t size, int maxEdge = 0);

private:
    Image Wrap(IRenderer::ImageHandle handle) const;

    std::unordered_map<std::string, Image> m_images;
};

} // namespace jwgui
