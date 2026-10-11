#pragma once
#include "../Core/UIRenderView.h"
#include "../Layout/LayoutNode.h"
#include <unordered_map>

namespace jwgui {

class JWGUI_API UIPanel : public UIRenderView {
public:
    UIPanel(float x, float y, float w, float h);
    virtual ~UIPanel() = default;

    void OnLayout() override;
    void BuildRenderItems() override;
    // The children as this panel's layout places them, plus its padding.
    bool MeasureContent(float& width, float& height) const override;

    void SetDrawBackground(bool draw);
    bool IsDrawBackground() const;

    // Layout configuration
    void SetLayoutType(jwgui::LayoutType type);
    std::shared_ptr<jwgui::LayoutNode> GetLayoutNode() const { return m_layoutRoot; }

    // Adds a child view and returns its automatically created LayoutNode for configuration.
    virtual std::shared_ptr<jwgui::LayoutNode> AddControl(std::shared_ptr<UIRenderView> child);
    // Like AddControl, placed before `before` (one of this panel's controls) in
    // drawing and layout order; at the end when `before` is not here.
    std::shared_ptr<jwgui::LayoutNode> InsertControl(std::shared_ptr<UIRenderView> child, UIRenderView* before);
    virtual std::shared_ptr<jwgui::LayoutNode> GetControlLayout(UIRenderView* child) const;

    // Overrides
    void AddChild(std::shared_ptr<UIRenderView> child) override;
    void RemoveChild(std::shared_ptr<UIRenderView> child) override;
    void ClearControls();

protected:
    void SyncBackgroundItem();

private:
    std::shared_ptr<jwgui::LayoutNode> m_layoutRoot;
    std::unordered_map<UIRenderView*, std::shared_ptr<jwgui::LayoutNode>> m_layoutMap;
    std::shared_ptr<UIRenderItem> m_bgItem;
    bool m_drawBackground = true;
};

} // namespace jwgui
