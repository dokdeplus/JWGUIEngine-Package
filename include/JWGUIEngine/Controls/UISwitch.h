#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include <string>

namespace jwgui {

class JWGUI_API UISwitch : public UIRenderView {
public:
    UISwitch(float x, float y, float w, float h);
    virtual ~UISwitch() = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;

    void SetTextVisible(bool visible);
    bool IsTextVisible() const;
    void SetOnText(const std::string& text);
    void SetOffText(const std::string& text);

    void SetChecked(bool checked);
    bool IsChecked() const;

    EventHandler<ToggleEventArgs> ToggleEvent;

private:
    bool m_isChecked = false;
    bool m_isTextVisible = false;
    std::string m_onText = "On";
    std::string m_offText = "Off";
    
    std::shared_ptr<UIRenderItem> m_trackItem;
    std::shared_ptr<UIRenderItem> m_handleItem;
    std::shared_ptr<class UIRenderText> m_textItem;

    void LayoutParts();
    void AnimateThumb();
};

} // namespace jwgui
