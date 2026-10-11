#pragma once

#include "UIPanel.h"
#include "../Core/Event.h"
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UITabControl : public UIPanel {
public:
    UITabControl(float x, float y, float w, float h);
    virtual ~UITabControl();

    void OnLayout() override;

    int AddTab(const std::string& id, const std::string& title);
    void ClearTabs();

    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return m_selected; }
    int TabCount() const { return static_cast<int>(m_tabs.size()); }
    const std::string& TabId(int index) const;
    // A tab's caption (a document was renamed, or changed: "name *").
    void SetTabTitle(int index, const std::string& title);

    void SetTrailing(std::shared_ptr<UIRenderView> view);
    void SetShowAddButton(bool show);
    bool GetShowAddButton() const { return m_showAddButton; }

    // False hides every tab's close button (a fixed set of pages).
    void SetTabsClosable(bool closable) { m_tabsClosable = closable; }
    bool GetTabsClosable() const { return m_tabsClosable; }

    void SetTabDragEnabled(bool enable) { m_tabDragEnabled = enable; }
    bool GetTabDragEnabled() const { return m_tabDragEnabled; }

    EventHandler<SelectionEventArgs> SelectionChangedEvent;
    EventHandler<SelectionEventArgs> TabCloseRequestedEvent;
    EventHandler<EventArgs> TabAddRequestedEvent;
    EventHandler<TabDragEventArgs> TabDragStartedEvent;

private:
    class TabItem;
    class TabAddButton;

    struct Tab {
        std::string id;
        std::shared_ptr<TabItem> item;
    };

    std::vector<Tab> m_tabs;
    int m_selected = -1;
    bool m_showAddButton = false;
    bool m_tabDragEnabled = false;
    bool m_tabsClosable = true;
    std::shared_ptr<TabAddButton> m_addButton;
    std::shared_ptr<UIRenderView> m_trailing;

    void DetachChrome();
    void AttachChrome();
    void EnsureAddButton();
    void PaintSelected();
    void OnTabClicked(TabItem* item);
    void OnTabCloseClicked(TabItem* item);
    void OnTabDragStarted(TabItem* item, float grabX, float grabY);
    void OnAddClicked();
};

} // namespace jwgui
