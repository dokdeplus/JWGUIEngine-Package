#pragma once

#include "UIPanel.h"
#include "UIStatic.h"
#include "../Layout/LayoutTypes.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIStatusBar : public UIPanel {
public:
    static constexpr float kThickness = 24.0f;

    UIStatusBar(float x, float y, float w, float h);
    ~UIStatusBar() override = default;

    void SetDock(DockSide side);
    DockSide GetDock() const { return m_dock; }

    void SetText(const std::string& text);
    const std::string& GetText() const;

    std::shared_ptr<UIStatic> AddPanel(const std::string& text, float width = 0.0f);

    float GetPreferredThickness() const { return kThickness; }

    void SetStyle(std::shared_ptr<UIStyle> style) override;

private:
    std::vector<std::shared_ptr<UIStatic>> m_panels;
    DockSide m_dock = DockSide::Bottom;

    void ApplyDockLayout();
};

} // namespace jwgui
