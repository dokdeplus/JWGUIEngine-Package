#pragma once

#include "../Core/Export.h"
#include <cstdint>
#include <string>
#include <vector>

namespace jwgui {

// A value band of a gauge: "60-85:warn". The name picks the style class
// (Type.zone-warn); a "#RRGGBB" / "#AARRGGBB" name is a literal color instead.
struct GaugeZone {
    double from = 0.0;
    double to = 0.0;
    std::string name;
    uint32_t color = 0;
    bool hasColor = false;

    bool Contains(double value) const { return value >= from && value <= to; }
};

// Value <-> position math shared by radial and linear gauges, the
// thermometer and dials. No UI types, so it is unit tested on its own.
class JWGUI_API GaugeScale {
public:
    GaugeScale() = default;
    GaugeScale(double minimum, double maximum) { SetRange(minimum, maximum); }

    // A reversed range is swapped; an empty one gets a span of 1.
    void SetRange(double minimum, double maximum);
    double Minimum() const { return m_min; }
    double Maximum() const { return m_max; }
    double Span() const { return m_max - m_min; }

    double Clamp(double value) const;
    // 0 at the minimum, 1 at the maximum, clamped.
    double ToT(double value) const;
    double FromT(double t) const;

    // Major tick values from the minimum to the maximum. intervals > 0 splits
    // the range evenly; 0 picks a 1-2-5 step that gives about 5-10 intervals.
    std::vector<double> MajorTicks(int intervals = 0) const;
    // perMajor - 1 values between each pair of majors (perMajor = 5 -> 4 minors).
    static std::vector<double> MinorTicks(const std::vector<double>& majors, int perMajor);

    // 1, 2 or 5 times a power of ten, so that span / step is close to target.
    static double NiceStep(double span, int target);
    // Decimals that show every value of a step without noise (0.25 -> 2).
    static int DecimalsFor(double step);
    static std::string Format(double value, int decimals);

    // Gauge angle for a value: start + t * sweep (degrees, clockwise).
    double AngleOf(double value, double startDeg, double sweepDeg) const {
        return startDeg + ToT(value) * sweepDeg;
    }

private:
    double m_min = 0.0;
    double m_max = 100.0;
};

// "0-60:normal; 60-85:warn; 85-120:#E24B4A". Bounds may be negative
// ("-20-0:cold") and are swapped when reversed; unreadable entries are skipped.
JWGUI_API std::vector<GaugeZone> ParseGaugeZones(const std::string& text);
// The last zone containing the value (later entries win), or null.
JWGUI_API const GaugeZone* FindGaugeZone(const std::vector<GaugeZone>& zones, double value);
// "#RRGGBB" or "#AARRGGBB" -> 0xAARRGGBB.
JWGUI_API bool ParseHexColor(const std::string& text, uint32_t& outColor);

} // namespace jwgui
