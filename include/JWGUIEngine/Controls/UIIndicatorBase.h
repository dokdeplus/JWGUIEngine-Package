#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/TimerQueue.h"
#include <string>

namespace jwgui {

// A lamp's (or a stack light tier's) state.
enum class LampState {
    Off,
    On,
    Blink   // the shared blink: every blinking indicator flips together
};

enum class LampShape {
    Circle,
    Square,  // rounded square
    Rect     // the whole box, label inside (an annunciator tile)
};

JWGUI_API LampState LampStateFromName(const std::string& name);  // "Off" / "On" / "Blink"
JWGUI_API const char* LampStateName(LampState state);

// What Lamp and StackLight share: one paint item, style-part lookups, the
// caption, glow, the lens drawing, and the blink tick. Blinking indicators
// repaint on the wall clock's half seconds, so they all flip at once.
//
// Colors: a color spec is "#RRGGBB" or a zone name: Type.zone-<name> /
// .zone-<name> (AccentColor), the same classes the gauges use (ok, warn,
// alarm, info, normal, or your own).
class JWGUI_API UIIndicatorBase : public UIRenderView {
public:
    UIIndicatorBase(float x, float y, float w, float h);
    ~UIIndicatorBase() override;

    void OnCreate() override;
    void OnLayout() override;
    void BuildRenderItems() override;

    void SetCaption(const std::string& caption);
    const std::string& GetCaption() const { return m_caption; }
    // Soft light around lit lenses.
    void SetGlow(bool glow);
    bool GetGlow() const { return m_glow; }

    // The lit half of the shared blink (on during even half seconds of the
    // wall clock).
    static bool BlinkPhaseOn();
    // The same for a wall clock time (ms since the epoch).
    static bool BlinkPhaseAt(long long wallMs) { return (wallMs / kBlinkHalfMs) % 2 == 0; }
    static constexpr unsigned int kBlinkHalfMs = 500;
    // Lit now: On, or Blink in its lit half.
    static bool IsLit(LampState state) { return state == LampState::On || (state == LampState::Blink && BlinkPhaseOn()); }

protected:
    // Draws everything inside (x, y, w, h).
    virtual void DrawIndicator(IRenderer* renderer, float x, float y, float w, float h) = 0;
    // Something blinks now: the view repaints on the blink boundaries.
    virtual bool NeedsBlink() const = 0;
    // Call when NeedsBlink may have changed.
    void UpdateBlinking();

    uint32_t PartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    // Only a rule for that part counts; else `fallback`.
    uint32_t NamedPartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    float PartFloat(const char* part, UIStylePropertyID prop, float fallback) const;
    // A part's Background as one color (a gradient's first stop).
    uint32_t PartFill(const char* part, uint32_t fallback) const;
    std::string FontFamily() const;
    // "#RRGGBB" / "#AARRGGBB", or a zone name (Type.zone-<name>); else fallback.
    uint32_t ResolveColor(const std::string& spec, uint32_t fallback) const;
    // The unlit lens: the `part` rule when set (.off), else `lit` darkened.
    uint32_t UnlitColor(const char* part, uint32_t lit) const;
    static uint32_t Shade(uint32_t color, float factor);  // < 1 darker, > 1 lighter
    // One lens in (x, y, w, h): lit (shaded like glass, glow when on) or unlit.
    void DrawLens(IRenderer* renderer, LampShape shape, float x, float y, float w, float h,
                  uint32_t litColor, uint32_t unlitColor, bool lit) const;

    std::string m_caption;
    bool m_glow = true;

private:
    class PaintItem;
    std::shared_ptr<PaintItem> m_paint;
    TimerId m_timer = 0;

    void LayoutParts();
    void StopBlinking();
};

} // namespace jwgui
