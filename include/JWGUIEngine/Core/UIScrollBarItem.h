#pragma once
#include "UIRenderItem.h"

namespace jwgui {

// A view's scroll bar: a track with a thumb. Styled as the `.scrollbar` part
// of the view (a `background` for the track; the thumb is the text color,
// faint until the pointer reaches it).
class JWGUI_API UIScrollBarItem : public UIRenderItem {
public:
    enum class Orientation {
        Horizontal,
        Vertical
    };

    UIScrollBarItem(Orientation orientation);
    virtual ~UIScrollBarItem() = default;

    // Viewport size vs Content size to determine thumb size
    void SetScrollMetrics(float viewportSize, float contentSize, float scrollOffset);

    // To hit test the thumb (x, y are relative to the scrollbar's m_rect)
    bool IsThumbHit(float x, float y) const;
    
    // Convert a mouse drag delta to scroll offset delta
    float DragToScrollDelta(float dragDelta) const;

    void UpdateCachedStyles(const std::shared_ptr<UIStyle>& style, const UIRenderView* view, UIState state,
                            bool animate = false,
                            const std::shared_ptr<UIRenderView>& owner = {},
                            const std::shared_ptr<UIRenderItem>& self = {}) override;

protected:
    virtual void OnDrawContent(IRenderer* renderer, float absX, float absY, float contentW, float contentH) override;

private:
    uint32_t m_cachedThumbColor = 0xFF808080;
    Orientation m_orientation;
    float m_viewportSize = 0.0f;
    float m_contentSize = 0.0f;
    float m_scrollOffset = 0.0f;
    
    // Cached thumb rect in local coordinates (relative to m_rect.x/y = 0/0)
    UIRect m_thumbRect;
};

} // namespace jwgui
