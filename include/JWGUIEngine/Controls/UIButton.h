#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include "../Core/Image.h"
#include <string>

namespace jwgui {

class JWGUI_API UIButton : public UIRenderView {
public:
    UIButton(float x, float y, float w, float h);
    virtual ~UIButton() = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetTextAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign);
    HorizontalAlignment GetTextHAlign() const { return m_textHAlign; }
    VerticalAlignment GetTextVAlign() const { return m_textVAlign; }

    // Caption plus icon (with its spacing) plus padding.
    bool MeasureContent(float& width, float& height) const override;

    void SetImage(const Image& image);
    void SetImageKey(const std::string& key);
    void ClearImage();
    const Image& GetImage() const { return m_image; }
    const std::string& GetImageKey() const { return m_imageKey; }

    // Left/Right/Top/Bottom: icon beside text. Center: icon centered (text overlays).
    // Absolute: SetImagePosition local top-left.
    void SetImagePlacement(ImagePlacement placement);
    ImagePlacement GetImagePlacement() const { return m_imagePlacement; }

    void SetImagePosition(float x, float y);
    void GetImagePosition(float& x, float& y) const;

    // 0 = decoded size.
    void SetImageSize(float width, float height);
    void GetImageSize(float& width, float& height) const;

    void SetImageSpacing(float spacing);
    float GetImageSpacing() const { return m_imageSpacing; }

    EventHandler<EventArgs> ClickEvent;

private:
    std::string m_text;
    std::string m_imageKey;
    Image m_image;
    ImagePlacement m_imagePlacement = ImagePlacement::Left;
    HorizontalAlignment m_textHAlign = HorizontalAlignment::Center;
    VerticalAlignment m_textVAlign = VerticalAlignment::Center;
    float m_imageX = 0.0f;
    float m_imageY = 0.0f;
    float m_imageWidth = 0.0f;
    float m_imageHeight = 0.0f;
    float m_imageSpacing = 6.0f;

    std::shared_ptr<class UIRenderItem> m_bgItem;
    std::shared_ptr<class UIRenderImage> m_imageItem;
    std::shared_ptr<class UIRenderText> m_textItem;

    void EnsureItems();
    void LayoutContent();
    Thickness ContentPadding() const;
    void ResolveImageSize(float& width, float& height) const;
};

} // namespace jwgui
