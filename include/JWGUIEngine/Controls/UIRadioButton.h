#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include <string>

namespace jwgui {

class JWGUI_API UIRadioButton : public UIRenderView {
public:
    UIRadioButton(float x, float y, float w, float h);
    virtual ~UIRadioButton() = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetChecked(bool checked);
    bool IsChecked() const;

    EventHandler<ToggleEventArgs> ToggleEvent;

private:
    std::string m_text;
    bool m_isChecked = false;
    float m_radioSize = 16.0f;
    float m_spacing = 4.0f;

    std::shared_ptr<UIRenderItem> m_radioItem;
    std::shared_ptr<class UIRenderText> m_textItem;
};

} // namespace jwgui
