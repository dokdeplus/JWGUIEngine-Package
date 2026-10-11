#pragma once

#include "UIPanel.h"
#include "UIButton.h"
#include "../Layout/LayoutTypes.h"
#include <string>

namespace jwgui {

class JWGUI_API UIToolBar : public UIPanel {
public:
    static constexpr float kThickness = 36.0f;

    UIToolBar(float x, float y, float w, float h);
    ~UIToolBar() override = default;

    void SetDock(DockSide side);
    DockSide GetDock() const { return m_dock; }
    bool IsHorizontal() const {
        return m_dock == DockSide::Top || m_dock == DockSide::Bottom;
    }

    std::shared_ptr<UIButton> AddButton(const std::string& text);
    std::shared_ptr<UIButton> AddButton(const std::string& text, const Image& image);
    void AddSeparator();

    float GetPreferredThickness() const { return kThickness; }

    void SetStyle(std::shared_ptr<UIStyle> style) override;

private:
    DockSide m_dock = DockSide::Top;

    void ApplyDockLayout();
};

} // namespace jwgui
