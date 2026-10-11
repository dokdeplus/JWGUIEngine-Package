#pragma once

#include "UIPanel.h"
#include "UIDockWidget.h"
#include "UITabControl.h"
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace jwgui {

enum class DockArea {
    Left,
    Right,
    Top,
    Bottom
};

enum class DockSplit {
    Horizontal, // children left-to-right
    Vertical    // children top-to-bottom
};

class JWGUI_API UIDockHost : public UIPanel {
public:
    UIDockHost(float x, float y, float w, float h);
    virtual ~UIDockHost();

    void OnLayout() override;
    void SetStyle(std::shared_ptr<UIStyle> style) override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    void OnMouseLeave() override;

    void SetCentralWidget(std::shared_ptr<UIRenderView> central);
    std::shared_ptr<UIRenderView> GetCentralWidget() const { return m_central; }

    void AddDockWidget(DockArea area, std::shared_ptr<UIDockWidget> widget);
    void SplitDockWidget(std::shared_ptr<UIDockWidget> first,
                         std::shared_ptr<UIDockWidget> second,
                         DockSplit split);
    void TabifyDockWidget(std::shared_ptr<UIDockWidget> first,
                          std::shared_ptr<UIDockWidget> second);
    void RemoveDockWidget(std::shared_ptr<UIDockWidget> widget);

    void SetDockVisible(std::shared_ptr<UIDockWidget> widget, bool visible);

    void BeginWidgetDrag(UIDockWidget* widget, float absX, float absY, float grabOffsetX, float grabOffsetY);
    void UpdateWidgetDrag(UIDockWidget* widget, float screenX, float screenY);
    void EndWidgetDrag(UIDockWidget* widget, float screenX, float screenY);
    void CancelWidgetDrag();

    void NotifyWidgetClosed(UIDockWidget* widget);
    void NotifyFloatClosed(UIDockWidget* widget);

    DockArea LastArea(UIDockWidget* widget) const;

    static constexpr float kSplitterSize = 5.0f;
    static constexpr float kDefaultBandSize = 240.0f;
    static constexpr float kMinBandSize = 80.0f;
    static constexpr float kMinCenterWidth = 240.0f;
    static constexpr float kMinCenterHeight = 160.0f;
    static constexpr float kTabBarHeight = 34.0f;

private:
    struct DockNode {
        bool isSplit = false;
        DockSplit split = DockSplit::Vertical;
        std::vector<std::unique_ptr<DockNode>> children;
        std::vector<float> ratios;
        std::vector<std::shared_ptr<UIDockWidget>> tabs;
        int selected = 0;
        UIRect rect;
        std::shared_ptr<UITabControl> tabBar;
    };

    enum class DropKind {
        None,
        Area,
        SplitBefore,
        SplitAfter,
        Tabify
    };

    struct DropTarget {
        DropKind kind = DropKind::None;
        DockArea area = DockArea::Left;
        DockNode* node = nullptr;
        DockSplit split = DockSplit::Horizontal;
        UIRect hint;
    };

    struct Splitter {
        UIRect rect;
        bool vertical = true; // true: resize width (left/right)
        int kind = 0; // 0-3 area sash, 4 node sash
        DockArea area = DockArea::Left;
        DockNode* node = nullptr;
        int childIndex = 0;
        std::shared_ptr<UIPanel> view;
    };

    std::shared_ptr<UIRenderView> m_central;
    std::unique_ptr<DockNode> m_areas[4];
    float m_bandSize[4] = { 200.0f, 270.0f, 160.0f, 160.0f };
    std::vector<std::shared_ptr<UIDockWidget>> m_widgets;
    std::unordered_map<UIDockWidget*, DockArea> m_lastAreas;
    std::vector<Splitter> m_splitters;
    size_t m_splitterWrite = 0;
    std::shared_ptr<UIPanel> m_hint;
    bool m_splitDragging = false;
    int m_splitKind = 0;
    DockArea m_splitArea = DockArea::Left;
    DockNode* m_splitNode = nullptr;
    int m_splitChildIndex = 0;
    bool m_splitVertical = true;
    // The sash cursor is showing. Set only on changes, so views under the
    // pointer (EditBox IBeam) keep their cursor the rest of the time.
    bool m_sashCursor = false;
    float m_splitDragStart = 0.0f;
    float m_splitDragSize = 0.0f;
    float m_splitDragSizeB = 0.0f;
    float m_splitDragAvail = 1.0f;
    float m_splitDragLiveA = 0.0f;
    UIDockWidget* m_dragWidget = nullptr;
    DropTarget m_drop;
    bool m_draggingWidget = false;
    bool m_syncingDockTabs = false;

    int AreaIndex(DockArea area) const { return static_cast<int>(area); }
    DockNode* AreaNode(DockArea area) const { return m_areas[AreaIndex(area)].get(); }

    void AttachWidget(const std::shared_ptr<UIDockWidget>& widget);
    void DetachFromTree(UIDockWidget* widget);
    std::unique_ptr<DockNode> MakeTabNode(std::shared_ptr<UIDockWidget> widget);
    DockNode* FindLeaf(DockNode* node, UIDockWidget* widget, DockNode** parent, int* index);
    bool NodeContains(DockNode* node, UIDockWidget* widget) const;
    void PruneNode(std::unique_ptr<DockNode>& node);
    void NormalizeRatios(DockNode* node);
    void CollectVisibleTabs(const DockNode* node, std::vector<std::shared_ptr<UIDockWidget>>& out) const;
    std::shared_ptr<UIDockWidget> SelectedTab(DockNode* node) const;
    void SelectTab(DockNode* node, UIDockWidget* widget);

    Size NodeMinSize(const DockNode* node) const;
    float AreaMinThickness(DockArea area) const;
    float ClampBandDrag(DockArea area, float desired) const;
    void DistributeSplitSizes(const DockNode* node, float avail, bool alongWidth, std::vector<float>& sizes) const;

    void LayoutArea(DockNode* node, const UIRect& rect);
    void LayoutSplit(DockNode* node, const UIRect& rect);
    void LayoutTabs(DockNode* node, const UIRect& rect);
    void ClearTransientViews();
    void AddSplitter(const UIRect& rect, bool vertical, int kind, DockArea area, DockNode* node, int childIndex);
    void TrimSplitters(size_t keep);
    void LayoutChangedChildren(const std::vector<std::pair<UIRenderView*, UIRect>>& before);
    void InvalidateSplitDirty(const std::vector<std::pair<UIRenderView*, UIRect>>& before);
    float SplitAvail(const DockNode* node, bool vertical) const;
    void BeginSplitterDrag(const Splitter& s, float x, float y);
    void ApplySplitterDrag(float x, float y);
    void EndSplitterDrag();
    void UpdateHint();
    DropTarget HitDrop(float localX, float localY) const;
    DropTarget HitDropOnNode(DockNode* node, float x, float y) const;
    void ApplyDrop(UIDockWidget* widget, const DropTarget& drop);
    void HostLocalFromScreen(float screenX, float screenY, float& localX, float& localY) const;
    void RebuildTabBar(DockNode* node);
    void BindDockTabBar(DockNode* node);
    void DetachTabBar(DockNode* node);
    void SweepOrphanTabBars();
    void RememberArea(UIDockWidget* widget, DockArea area);
    bool AreaHasVisible(DockArea area) const;
    float VisibleBand(DockArea area) const;
};

} // namespace jwgui
