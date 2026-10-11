#pragma once
#include "Export.h"
#include "IRenderer.h"
#include <memory>

namespace jwgui {

struct ImageData;

// Shared RAII image. Copies share the decoded handle; the last copy FreeImage's.
// Independent of ImageManager — create via ImageManager or wrap a handle yourself.
class JWGUI_API Image {
public:
    Image();
    Image(const Image& other);
    Image(Image&& other) noexcept;
    Image& operator=(const Image& other);
    Image& operator=(Image&& other) noexcept;
    ~Image();

    bool IsValid() const;
    IRenderer::ImageHandle Handle() const;
    float Width() const;
    float Height() const;
    void GetSize(float& width, float& height) const;
    void Reset();

private:
    friend class ImageManager;
    static Image Adopt(IRenderer::ImageHandle handle);
    std::shared_ptr<ImageData> m_data;
};

} // namespace jwgui
