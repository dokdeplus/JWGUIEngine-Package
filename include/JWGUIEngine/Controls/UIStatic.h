#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/UIRenderText.h"
#include "../Core/IRenderer.h"
#include "../Core/Image.h"
#include <cstddef>
#include <string>

namespace jwgui {

class JWGUI_API UIStatic : public UIRenderView {
public:
    UIStatic(float x, float y, float w, float h);
    ~UIStatic() override = default;

    void OnLayout() override;
    void BuildRenderItems() override;

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetTextAlignment(jwgui::UIRenderText::HAlign hAlign, jwgui::UIRenderText::VAlign vAlign);

    void SetWordWrap(bool wrap);
    bool IsWordWrap() const { return m_wordWrap; }

    // Text lines (or the picture's own size) plus padding.
    bool MeasureContent(float& width, float& height) const override;

    void SetImagePath(const std::string& path);
    const std::string& GetImagePath() const;

    void SetImage(const Image& image);
    const Image& GetImage() const { return m_image; }

    void SetImageStretch(ImageStretch stretch);
    ImageStretch GetImageStretch() const { return m_imageStretch; }
    bool SetImageFromMemory(const void* data, size_t size);

private:
    std::string m_text;
    std::string m_imagePath;
    ImageStretch m_imageStretch = ImageStretch::Stretch;
    Image m_image;
    bool m_wordWrap = false;
    
    std::shared_ptr<class UIRenderText> m_textItem;
    std::shared_ptr<class UIRenderImage> m_imageItem;

    void RefreshWrappedContent();
    void ReleaseImage();
    void LoadImageFromPath();
    void SyncImageItem();
    bool HasImage() const { return m_image.IsValid(); }
};

} // namespace jwgui
