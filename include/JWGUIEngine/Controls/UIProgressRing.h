#pragma once

#include "UIGaugeBase.h"
#include "../Core/TimerQueue.h"

namespace jwgui {

// A ring that fills clockwise from 12 o'clock to the value, the value in the
// middle; or, indeterminate, a turning arc for "busy". Scene type
// `ProgressRing` (Toolbox Extended › GAUGES). Range, value (it glides),
// decimals, unit (default %), zones and ZoneChangedEvent come from
// UIGaugeBase; a zone colors the ring.
//
// Style parts (Type.part): ProgressRing (box, bare), .track, .fill (the
// arc), .value, .unit, .caption (under the value).
class JWGUI_API UIProgressRing : public UIGaugeBase {
public:
    UIProgressRing(float x, float y, float w, float h);
    ~UIProgressRing() override;

    void OnCreate() override;

    // A turning arc instead of the value (unknown progress).
    void SetIndeterminate(bool indeterminate);
    bool GetIndeterminate() const { return m_indeterminate; }
    // Ring width, percent of the radius (2..60).
    void SetThickness(double percent);
    double GetThickness() const { return m_thickness; }
    // Round ends on the arc.
    void SetRoundCaps(bool round);
    bool GetRoundCaps() const { return m_roundCaps; }

    // Where the turning arc starts now (clock degrees), from the time.
    static float SpinAngle();

protected:
    void DrawGauge(IRenderer* renderer, float x, float y, float w, float h) override;

private:
    bool m_indeterminate = false;
    double m_thickness = 12.0;
    bool m_roundCaps = true;
    TimerId m_spinTimer = 0;

    void UpdateSpinning();
    void StopSpinning();
};

} // namespace jwgui
