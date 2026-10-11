#pragma once

#include "UIGaugeBase.h"
#include "PartImage.h"

namespace jwgui {

enum class RadialGaugeStyle {
    Needle,  // ticks, labels and a needle over a track with zone bands
    Arc      // a thick arc that fills to the value, value in the middle
};

// Round dial: RPM, speed, pressure, load. Scene type `RadialGauge`.
// Angles are clock angles: 0 = 12 o'clock, clockwise. The default 225 / 270
// runs from 7:30 round to 4:30, leaving the gap at the bottom for the value.
//
// Extra style parts: RadialGauge.face (Background / OutlineColor /
// OutlineThickness), RadialGauge.needle (ForegroundColor).
class JWGUI_API UIRadialGauge : public UIGaugeBase {
public:
    UIRadialGauge(float x, float y, float w, float h);

    void SetStartAngle(double degrees);
    double GetStartAngle() const { return m_startAngle; }
    void SetSweepAngle(double degrees);
    double GetSweepAngle() const { return m_sweep; }

    void SetGaugeStyle(RadialGaugeStyle style);
    RadialGaugeStyle GetGaugeStyle() const { return m_gaugeStyle; }
    void SetGaugeStyleName(const std::string& name);  // "Needle" / "Arc"

    // Pictures (Project resource ids) instead of the drawn face and needle.
    // The needle picture stands pointing at 12 o'clock; NeedlePivot is its
    // point on the center ("0.5,0.85"), and the part above it reaches the
    // track. Empty = drawn.
    void SetFaceImage(const std::string& id);
    void SetNeedleImage(const std::string& id);
    void SetNeedlePivot(const std::string& pivot);

protected:
    void DrawGauge(IRenderer* renderer, float x, float y, float w, float h) override;

    // Clock angle of a value (degrees, 0 = 12 o'clock).
    double AngleOf(double value) const { return m_scale.AngleOf(value, m_startAngle, m_sweep); }

private:
    double m_startAngle = 225.0;
    double m_sweep = 270.0;
    RadialGaugeStyle m_gaugeStyle = RadialGaugeStyle::Needle;
    PartImage m_faceImage;
    PartImage m_needleImage;
    float m_pivotX = 0.5f;
    float m_pivotY = 0.85f;
};

} // namespace jwgui
