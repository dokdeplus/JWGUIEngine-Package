#pragma once
#include "Export.h"
#include "IRenderer.h"
#include "CoreTypes.h"
#include "UIStyle.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace jwgui {

/// <summary>
/// UIRenderItem represents the smallest unit of drawing.
/// It uses a UIStyle to query properties like Background based on its state.
/// </summary>
class JWGUI_API UIRenderItem {
public:
    virtual ~UIRenderItem() = default;

    UIRect m_rect = {0.0f, 0.0f, 0.0f, 0.0f};
    UIState m_state = UIState::Normal;
    bool m_followsViewState = true;
    // Drawn in the view's own frame instead of scrolling with its content
    // (a panel's background and border). Row items leave it false.
    bool m_viewSpace = false;

    /// <summary>
    /// Updates cached style properties from the provided style.
    /// </summary>
    virtual void UpdateCachedStyles(const std::shared_ptr<UIStyle>& style, const UIRenderView* view, UIState state,
                                    bool animate = false,
                                    const std::shared_ptr<UIRenderView>& owner = {},
                                    const std::shared_ptr<UIRenderItem>& self = {});

    virtual void SetAnimatedColor(UIStylePropertyID prop, uint32_t color);

    /// <summary>
    /// Background + content + outline. Prefer RenderBody/RenderOutline when children
    /// should paint between the fill and the border.
    /// </summary>
    virtual void Render(IRenderer* renderer, float offsetX, float offsetY);
    void RenderBody(IRenderer* renderer, float offsetX, float offsetY);
    void RenderOutlineOnly(IRenderer* renderer, float offsetX, float offsetY);

    // Box painting, shared with views that paint their own frame (EditBox,
    // Calendar): the fill of `box`, then its border. A rounded border has one
    // thickness: the left side's, or `roundedWidth` when the sides differ.
    static void PaintBackground(IRenderer* renderer, const UIRect& box, const UIBackground& background,
                                const CornerRadius& radius);
    static void PaintOutline(IRenderer* renderer, const UIRect& box, uint32_t color, const Thickness& width,
                             float roundedWidth, const CornerRadius& radius);

    /// <summary>
    /// Gets a cached property value. Returns false if not found.
    /// </summary>
    bool GetCachedProperty(UIStylePropertyID prop, UIStylePropertyValue& outVal) const;
    // A cached color, or `fallback` when the cache has none.
    uint32_t CachedColor(UIStylePropertyID prop, uint32_t fallback) const;

    void SetId(const std::string& id) { m_id = id; }
    const std::string& GetId() const { return m_id; }

    void SetClassName(const std::string& className) { m_className = className; }
    const std::string& GetClassName() const { return m_className; }

    /// <summary>
    /// Builds a style query from this item and its owning view.
    /// Item id/class take priority; type comes from the view.
    /// </summary>
    UIStyleQuery MakeStyleQuery(const UIRenderView* view, UIState state) const;

    void SetCachedProperty(UIStylePropertyID prop, const UIStylePropertyValue& value);
    void EraseCachedProperty(UIStylePropertyID prop);

protected:
    // A handful of values per item (a list row is an item, so thousands of
    // them exist): a short list beats a hash map here.
    std::vector<std::pair<UIStylePropertyID, UIStylePropertyValue>> m_cachedProperties;
    std::string m_id;
    std::string m_className;

    CornerRadius GetCachedCornerRadius() const;
    void DrawBackground(IRenderer* renderer, float x, float y, const CornerRadius& radius);
    void DrawOutline(IRenderer* renderer, float x, float y, const CornerRadius& radius);

    virtual void OnDrawContent(IRenderer* renderer, float absX, float absY, float contentW, float contentH) {}

    // Whatever else the item draws with (an accent, a radius), looked up here
    // with the box properties and kept until the next update. Nothing is
    // queried while painting.
    virtual void OnCacheStyles(const UIStyle& style, const UIStyleQuery& query) {
        (void)style;
        (void)query;
    }
};

} // namespace jwgui
