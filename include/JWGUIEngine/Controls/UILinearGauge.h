#pragma once

#include "UIGaugeBase.h"

namespace jwgui {

enum class LinearGaugeStyle {
    Fill,     // a bar that fills to the value
    Pointer   // a marker on the scale
};

// Straight scale: tank level, flow, load bar. Scene type `LinearGauge`.
// Horizontal: caption and value above the bar, zone strip and ticks below.
// Vertical: value on top, bar on the left, ticks and labels to its right,
// caption at the bottom.
//
// Extra style part: LinearGauge.pointer (ForegroundColor).
class JWGUI_API UILinearGauge : public UIGaugeBase {
public:
    UILinearGauge(float x, float y, float w, float h);

    void SetVertical(bool vertical);
    bool IsVertical() const { return m_vertical; }
    void SetOrientationName(const std::string& name);  // "Horizontal" / "Vertical"

    void SetGaugeStyle(LinearGaugeStyle style);
    LinearGaugeStyle GetGaugeStyle() const { return m_gaugeStyle; }
    void SetGaugeStyleName(const std::string& name);  // "Fill" / "Pointer"

protected:
    void DrawGauge(IRenderer* renderer, float x, float y, float w, float h) override;

private:
    bool m_vertical = false;
    LinearGaugeStyle m_gaugeStyle = LinearGaugeStyle::Fill;

    void DrawHorizontal(IRenderer* renderer, float x, float y, float w, float h);
    void DrawVertical(IRenderer* renderer, float x, float y, float w, float h);
};

} // namespace jwgui
