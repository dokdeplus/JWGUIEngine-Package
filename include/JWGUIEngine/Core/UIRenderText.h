#pragma once
#include "UIRenderItem.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIRenderText : public UIRenderItem {
public:
    UIRenderText() = default;
    ~UIRenderText() override = default;

    enum class HAlign { Left, Center, Right };
    enum class VAlign { Top, Middle, Bottom };

    std::string m_text;
    std::string m_fontFamily = UIStyle::DefaultFontFamily();
    HAlign m_hAlign = HAlign::Left;
    VAlign m_vAlign = VAlign::Middle;
    bool m_wordWrap = false;

    void UpdateCachedStyles(const std::shared_ptr<UIStyle>& style, const UIRenderView* view, UIState state,
                            bool animate = false,
                            const std::shared_ptr<UIRenderView>& owner = {},
                            const std::shared_ptr<UIRenderItem>& self = {}) override;
    void SetAnimatedColor(UIStylePropertyID prop, uint32_t color) override;

    void WrapLines(IRenderer* renderer, float maxWidth, std::vector<std::string>& out) const;
    float LineHeight() const { return m_cachedFontSize * 1.2f; }
    float CachedFontSize() const { return m_cachedFontSize; }
    const std::string& CachedFontFamily() const { return m_cachedFontFamily; }

protected:
    void OnDrawContent(IRenderer* renderer, float absX, float absY, float contentW, float contentH) override;

    uint32_t m_cachedTextColor = 0xFF000000;
    float m_cachedFontSize = 14.0f;
    std::string m_cachedFontFamily = UIStyle::DefaultFontFamily();

private:
    // The lines of the last paint, kept while the text, the width it was
    // wrapped to and the font stay the same: wrapping measures every
    // character, which is too much for every frame.
    struct LineCache {
        std::string text;
        float width = -1.0f;
        bool wordWrap = false;
        std::string fontFamily;
        float fontSize = 0.0f;
        float lineHeight = 0.0f;
        std::vector<std::string> lines;
    };
    const LineCache& LinesFor(IRenderer* renderer, float contentW);
    LineCache m_lineCache;
};

} // namespace jwgui
