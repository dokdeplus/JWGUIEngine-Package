#pragma once

#include "UIPanel.h"

namespace jwgui {

enum class SplitterOrientation {
    Horizontal,
    Vertical
};

class JWGUI_API UISplitter : public UIPanel {
public:
    static constexpr float kSashSize = 5.0f;
    static constexpr float kMinPane = 24.0f;

    UISplitter(float x, float y, float w, float h);
    ~UISplitter() override = default;

    void SetOrientation(SplitterOrientation orientation);
    SplitterOrientation GetOrientation() const { return m_orientation; }

    void SetRatio(float ratio);
    float GetRatio() const { return m_ratio; }

    std::shared_ptr<UIPanel> GetPane(int index) const;

    std::shared_ptr<LayoutNode> AddControl(std::shared_ptr<UIRenderView> child) override;
    std::shared_ptr<LayoutNode> GetControlLayout(UIRenderView* child) const override;
    void SetStyle(std::shared_ptr<UIStyle> style) override;

    void OnLayout() override;
    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    void OnMouseLeave() override;

private:
    SplitterOrientation m_orientation = SplitterOrientation::Horizontal;
    float m_ratio = 0.5f;
    bool m_building = false;
    bool m_dragging = false;
    bool m_sashCursor = false;
    float m_dragStart = 0.0f;
    float m_dragPane = 0.0f;

    std::shared_ptr<UIPanel> m_pane0;
    std::shared_ptr<UIPanel> m_pane1;
    std::shared_ptr<UIPanel> m_sash;

    void BuildChrome();
    void ApplySplitLayout();
    bool HitSash(float x, float y) const;
    void UpdateRatioFromPointer(float x, float y);
    void ApplyCursor(bool overSash);
    int PaneContentCount(const std::shared_ptr<UIPanel>& pane) const;
};

} // namespace jwgui
