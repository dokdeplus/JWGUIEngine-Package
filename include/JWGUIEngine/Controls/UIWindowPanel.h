#pragma once

#include "UIPanel.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "../Core/Event.h"
#include <string>

namespace jwgui {

class JWGUI_API UIWindowPanel : public UIPanel {
public:
    UIWindowPanel(float x, float y, float w, float h);
    virtual ~UIWindowPanel() = default;

    void SetTitle(const std::string& title);
    std::string GetTitle() const { return m_title; }

    // Toggle buttons
    void SetCloseBox(bool show);
    void SetMaximizeBox(bool show);
    void SetMinimizeBox(bool show);

    void SetStyle(std::shared_ptr<UIStyle> style) override;

    // Get Content Panel to add custom UI
    std::shared_ptr<UIPanel> GetContentPanel() const { return m_contentPanel; }

    // Events
    EventHandler<EventArgs> CloseEvent;
    EventHandler<EventArgs> MaximizeEvent;
    EventHandler<EventArgs> MinimizeEvent;

protected:
    void RebuildTitleBar();

    std::string m_title;
    bool m_showClose = true;
    bool m_showMaximize = false;
    bool m_showMinimize = false;

    std::shared_ptr<UIPanel> m_titleBar;
    std::shared_ptr<UIStatic> m_titleLabel;
    std::shared_ptr<UIPanel> m_buttonContainer;
    std::shared_ptr<jwgui::LayoutNode> m_btnContainerNode;
    
    std::shared_ptr<UIButton> m_btnClose;
    std::shared_ptr<UIButton> m_btnMaximize;
    std::shared_ptr<UIButton> m_btnMinimize;

    std::shared_ptr<UIPanel> m_contentPanel;
};

} // namespace jwgui
