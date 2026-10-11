#pragma once
#include "UIRenderItem.h"

namespace jwgui {

class JWGUI_API UIRenderImage : public UIRenderItem {
public:
    UIRenderImage() = default;
    ~UIRenderImage() override = default;

    void SetImage(IRenderer::ImageHandle image);
    IRenderer::ImageHandle GetImage() const;

    void SetStretch(ImageStretch stretch);
    ImageStretch GetStretch() const { return m_stretch; }

protected:
    void OnDrawContent(IRenderer* renderer, float absX, float absY, float contentW, float contentH) override;

private:
    IRenderer::ImageHandle m_image = nullptr;
    ImageStretch m_stretch = ImageStretch::Stretch;
};

} // namespace jwgui
