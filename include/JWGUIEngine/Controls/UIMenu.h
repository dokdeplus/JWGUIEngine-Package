#pragma once
#include "UIPopup.h"
#include "../Core/Event.h"
#include "../Core/UIRenderText.h"
#include <functional>
#include <string>
#include <vector>

namespace jwgui {

class UIMenuBar;

class JWGUI_API UIMenu : public UIPopup {
public:
    static constexpr float kHoverUnlockDistance = 3.0f;
    static constexpr float kPadding = 2.0f;
    static constexpr float kMinWidth = 120.0f;

    UIMenu();
    virtual ~UIMenu() = default;

    void AddItem(const std::string& text);
    // shortcut is shown dimmed on the right ("Ctrl+C"); it is only a label,
    // the app handles the key. ItemClickedEvent still gets text.
    void AddItem(const std::string& text, const std::string& shortcut);
    // An item whose label may be translated: ItemClickedEvent and
    // SetItemChecked use `command` (a plain AddItem's command is its text).
    void AddCommand(const std::string& command, const std::string& label, const std::string& shortcut = "");
    // Check mark in front of an item (e.g. the current view option). The menu
    // keeps a column for marks once any item has been set.
    void SetItemChecked(const std::string& command, bool checked);
    void AddSeparator();
    void AddSubMenu(const std::string& text, std::shared_ptr<UIMenu> subMenu);
    void RemoveItem(int index);
    void ClearItems();

    void ShowMenu(std::shared_ptr<UIRenderView> parentView, float x, float y);
    void HideMenu();
    void CloseSubtree();

    bool HitAbsolute(float absX, float absY) const;

    EventHandler<std::string> ItemClickedEvent;

    void SetActiveSubMenu(std::shared_ptr<UIMenu> subMenu);
    std::shared_ptr<UIMenu> GetActiveSubMenu() const { return m_activeSubMenu; }

    void SetParentMenu(UIMenu* parent) { m_parentMenu = parent; }
    UIMenu* GetParentMenu() const { return m_parentMenu; }

    void SetMenuBar(UIMenuBar* bar);
    UIMenuBar* GetMenuBar() const { return m_menuBar; }

    void SetFlyoutLeft(bool flyoutLeft) { m_flyoutLeft = flyoutLeft; }
    bool GetFlyoutLeft() const { return m_flyoutLeft; }

    void RecalculateSize();

    void OnLayout() override;
    void BuildRenderItems() override;
    void OnMouseLeave() override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    void OnGlobalMouseDown(float x, float y) override;

private:
    class UIMenuItem : public UIRenderText {
    public:
        enum class Type {
            Normal,
            Separator,
            SubMenu
        };

        static constexpr float kItemHeight = 25.0f;
        static constexpr float kSeparatorHeight = 9.0f;
        static constexpr float kHorizontalPadding = 10.0f;
        static constexpr float kArrowWidth = 16.0f;
        static constexpr float kCheckWidth = 18.0f;
        static constexpr float kShortcutGap = 28.0f;

        Type type = Type::Normal;
        std::string command;  // ItemClickedEvent; the text unless AddCommand
        std::string shortcut;
        bool checked = false;
        bool checkColumn = false; // set by the menu when any item can be checked
        std::shared_ptr<UIMenu> subMenu;
        std::function<void()> Click;

        float GetPreferredWidth() const;
        float GetPreferredHeight() const;

    protected:
        void OnDrawContent(IRenderer* renderer, float absX, float absY, float contentW, float contentH) override;

    private:
        float MeasureLabelWidth() const;
        float MeasureWidth(const std::string& text) const;
    };

    std::shared_ptr<UIRenderItem> m_bgItem;
    std::vector<std::shared_ptr<UIMenuItem>> m_items;
    std::shared_ptr<UIMenu> m_activeSubMenu;
    UIMenu* m_parentMenu = nullptr;
    UIMenuBar* m_menuBar = nullptr;
    bool m_flyoutLeft = false;
    bool m_checkColumn = false;
    int m_hoveredIndex = -1;
    int m_openSubMenuIndex = -1;
    bool m_hoverLocked = false;
    bool m_hoverLockPosValid = false;
    float m_hoverLockX = 0.0f;
    float m_hoverLockY = 0.0f;

    std::shared_ptr<UIMenuItem> CreateItem(const std::string& text, UIMenuItem::Type type);
    void LayoutItems();
    void SyncRenderItems();
    void ApplyItemStates();
    void ClearItemHover();
    int HitTestItem(float x, float y) const;
    UIState ItemVisualState(int index) const;
    void UpdateHover(int index);
    void LockItemHoverUntilMouseMove();
    bool ShouldSuppressItemHover(float x, float y);
    void ShowSubMenuFor(int index);
    void OnLeafItemClicked(std::shared_ptr<UIMenuItem> item);
    UIMenu* GetRootMenu();
};

} // namespace jwgui
