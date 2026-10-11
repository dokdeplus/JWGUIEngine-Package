#pragma once
#include "UIPanel.h"
#include "UIMenu.h"
#include "../Core/UIRenderText.h"
#include "../Layout/LayoutTypes.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIMenuBar : public UIPanel {
public:
    static constexpr float kHorizontalThickness = 28.0f;
    static constexpr float kVerticalItemHeight = 28.0f;
    static constexpr float kLabelPadding = 10.0f;

    UIMenuBar(float x, float y, float w, float h);
    virtual ~UIMenuBar() = default;

    void SetDock(DockSide side);
    DockSide GetDock() const { return m_dock; }

    bool IsHorizontal() const {
        return m_dock == DockSide::Top || m_dock == DockSide::Bottom;
    }

    std::shared_ptr<UIMenu> AddMenu(const std::string& text);
    void RemoveMenu(int index);
    void ClearMenus();

    bool IsTracking() const { return m_tracking; }
    void Dismiss();
    void TrackPointerAbsolute(float absX, float absY);

    float GetPreferredThickness() const;
    bool HitAbsolute(float absX, float absY) const;

    void OnLayout() override;
    void BuildRenderItems() override;
    void OnMouseLeave() override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseDown(float x, float y, MouseButton button) override;

private:
    class UIMenuBarItem : public UIRenderText {
    public:
        std::shared_ptr<UIMenu> menu;
        float GetPreferredWidth() const;

    private:
        float MeasureLabelWidth() const;
    };

    DockSide m_dock = DockSide::Top;
    bool m_tracking = false;
    std::shared_ptr<UIMenu> m_activeMenu;
    std::shared_ptr<UIRenderItem> m_bgItem;
    std::vector<std::shared_ptr<UIMenuBarItem>> m_items;
    int m_hoveredIndex = -1;
    int m_selectedIndex = -1;

    void LayoutItems();
    void SyncRenderItems();
    void ApplyItemStates();
    void ClearItemHover();
    int HitTestItem(float x, float y) const;
    UIState ItemVisualState(int index) const;
    void OpenMenu(int index);
    void ClearItemSelection();
};

} // namespace jwgui
