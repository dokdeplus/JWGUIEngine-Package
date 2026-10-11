#pragma once
#include "UIPanel.h"
#include "UIButton.h"
#include "../Core/Event.h"
#include <memory>
#include <string>

namespace jwgui {

// A titled section that folds its content away. Clicking the title row (a
// button of style type ExpanderHeader, with a chevron) toggles it.
//
// Put it in a Flex Column. Layout has no content-sized Auto yet, so the
// expander sets its own height in the parent: the title, plus the content rows
// while expanded. Call SyncSize() once after adding it to the parent; AddItem
// and SetExpanded keep it in sync after that.
class JWGUI_API UIExpander : public UIPanel {
public:
    static constexpr float kHeaderHeight = 28.0f;

    UIExpander(float x, float y, float w, float h);
    virtual ~UIExpander() = default;

    void SetHeader(const std::string& text);
    const std::string& GetHeader() const;
    std::shared_ptr<UIButton> GetHeaderButton() const { return m_header; }

    void SetExpanded(bool expanded);
    bool IsExpanded() const { return m_expanded; }
    void Toggle() { SetExpanded(!m_expanded); }

    // Content rows stack top to bottom at a fixed height (margin included in
    // the total). Returns the row's layout slot.
    std::shared_ptr<LayoutNode> AddItem(std::shared_ptr<UIRenderView> item, float height,
                                        const Thickness& margin = Thickness());
    std::shared_ptr<UIPanel> GetContent() const { return m_content; }

    // Title, plus the content rows while expanded.
    float GetDesiredHeight() const;
    // Writes GetDesiredHeight into the parent's slot and lays the parent out.
    void SyncSize();

    // Also styles the title and content panel (not the rows; give them a style).
    void SetStyle(std::shared_ptr<UIStyle> style) override;
    void RenderContent(IRenderer* renderer, float absX, float absY) override;

    // isChecked = expanded.
    EventHandler<ToggleEventArgs> ExpandedChangedEvent;

private:
    std::shared_ptr<UIButton> m_header;
    std::shared_ptr<UIPanel> m_content;
    std::shared_ptr<LayoutNode> m_contentSlot;
    bool m_expanded = true;

    float ContentHeight() const;
};

} // namespace jwgui
