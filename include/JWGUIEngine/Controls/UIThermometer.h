#pragma once

#include "UIGaugeBase.h"

namespace jwgui {

// Glass thermometer: a tube and bulb with a column that rises to the value,
// ticks and labels on the right, the value and unit on top, a caption at the
// bottom. Scene type `Thermometer`. The column takes the zone color, so
// "60-85:warn; 85-120:alarm" turns it amber, then red, as it heats up.
//
// Extra style part: Thermometer.tube (Background inside the glass,
// OutlineColor / OutlineThickness the glass). The column is Thermometer.fill
// (accent) until the value enters a zone, so it can turn amber, then red.
class JWGUI_API UIThermometer : public UIGaugeBase {
public:
    UIThermometer(float x, float y, float w, float h);

protected:
    void DrawGauge(IRenderer* renderer, float x, float y, float w, float h) override;
};

} // namespace jwgui
