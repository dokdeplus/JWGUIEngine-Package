#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include "GaugeScale.h"
#include <string>
#include <vector>

namespace jwgui {

// Common part of the gauges (UIRadialGauge, UILinearGauge, UIThermometer):
// range, value (the needle or fill tweens to it), ticks, unit, zones with
// ZoneChangedEvent, set point, caption, and style-part lookups. A subclass
// only draws (DrawGauge); apps subclass the concrete gauges and override
// FormatValue / OnZoneChanged / DrawGauge.
//
// Style parts shared by all gauges (Type.part): Type (box, bare by default),
// .track, .fill, .tick, .label, .value, .unit, .caption, .setpoint
// (ForegroundColor), and Type.zone-<name> / .zone-<name> (AccentColor) for
// zone colors.
class JWGUI_API UIGaugeBase : public UIRenderView {
public:
    UIGaugeBase(float x, float y, float w, float h);
    ~UIGaugeBase() override;

    void OnCreate() override;
    void OnLayout() override;
    void BuildRenderItems() override;

    void SetMinimum(double minimum);
    void SetMaximum(double maximum);
    void SetRange(double minimum, double maximum);
    double GetMinimum() const { return m_scale.Minimum(); }
    double GetMaximum() const { return m_scale.Maximum(); }
    const GaugeScale& GetScale() const { return m_scale; }

    // The value may lie outside the range: the text shows it, the needle /
    // fill stops at the end. Shown values tween when animate is on.
    void SetValue(double value);
    double GetValue() const { return m_value; }
    double GetDisplayedValue() const { return m_shown; }
    void SetAnimate(bool animate) { m_animate = animate; }
    bool GetAnimate() const { return m_animate; }

    void SetUnit(const std::string& unit);
    const std::string& GetUnit() const { return m_unit; }
    // Digits after the point for labels and the value; -1 = from the tick step.
    void SetDecimals(int decimals);
    int GetDecimals() const { return m_decimals; }
    // The locale's decimal mark (3,5 in de-DE), or always '.' (Invariant).
    void SetLocaleNumbers(bool locale);
    bool GetLocaleNumbers() const { return m_localeNumbers; }
    void SetNumbersName(const std::string& name);  // "Locale" / "Invariant"
    // Major tick intervals; 0 = a 1-2-5 step. Minor: divisions per major (< 2 = none).
    void SetMajorTicks(int intervals);
    void SetMinorTicks(int perMajor);

    // "0-60:normal; 60-85:warn; 85-100:alarm" (see ParseGaugeZones).
    void SetZones(const std::string& zones);
    const std::vector<GaugeZone>& GetZones() const { return m_zones; }
    const std::string& GetZonesText() const { return m_zonesText; }
    // Zone of the current value, or null.
    const GaugeZone* CurrentZone() const { return FindGaugeZone(m_zones, m_value); }
    // Value fill / arc / column takes its zone's color (otherwise Type.fill).
    void SetColorByZone(bool on);
    bool GetColorByZone() const { return m_colorByZone; }
    // The zone colors drawn beside the scale (bands on the dial, strips by
    // the bar or tube). Off hides them; the zones still color the value.
    void SetShowZoneBands(bool show);
    bool GetShowZoneBands() const { return m_showZoneBands; }

    // A target marker. Scene text: a number, or empty for none.
    void SetSetPointText(const std::string& text);
    void SetSetPoint(double value);
    void ClearSetPoint();
    bool HasSetPoint() const { return m_hasSetPoint; }
    double GetSetPoint() const { return m_setPoint; }

    void SetShowValue(bool show);
    bool GetShowValue() const { return m_showValue; }
    void SetCaption(const std::string& caption);
    const std::string& GetCaption() const { return m_caption; }

    // The zone the value is in now, by name ("" = outside all zones).
    const std::string& GetZoneName() const { return m_zoneName; }
    // The value crossed into another zone (by name; "" = outside all zones).
    EventHandler<GaugeZoneEventArgs> ZoneChangedEvent;

protected:
    // Value text without the unit.
    virtual std::string FormatValue(double value) const;
    // A value or scale label with `decimals` places and the decimal mark
    // SetLocaleNumbers picks.
    std::string FormatNumber(double value, int decimals) const;
    // Runs before ZoneChangedEvent.
    virtual void OnZoneChanged(const GaugeZoneEventArgs& args) { (void)args; }
    // Draws everything inside (x, y, w, h).
    virtual void DrawGauge(IRenderer* renderer, float x, float y, float w, float h) = 0;

    // "Type.part" lookups (with the base types of an app subclass).
    uint32_t PartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    float PartFloat(const char* part, UIStylePropertyID prop, float fallback) const;
    std::string FontFamily() const;
    uint32_t ZoneColor(const GaugeZone& zone, uint32_t fallback) const;
    // Fill color for a value: its zone's (colorByZone) or Type.fill.
    uint32_t ValueColor(double value) const;
    // Value text color: the zone's when the zone is not "normal" / "ok".
    uint32_t ValueTextColor(double value) const;
    // A round face from Type.face: solid or gradient (radial) fill and a rim.
    void DrawRoundFace(IRenderer* renderer, float cx, float cy, float radius) const;

    int EffectiveDecimals() const;
    // Zone bands are drawn: some zones and showZoneBands on.
    bool ShowsZoneBands() const { return m_showZoneBands && !m_zones.empty(); }
    // The zones to draw as bands: all of them, or none when bands are off.
    const std::vector<GaugeZone>& BandZones() const {
        static const std::vector<GaugeZone> kNone;
        return ShowsZoneBands() ? m_zones : kNone;
    }
    std::vector<double> MajorTicks() const { return m_scale.MajorTicks(m_majorTicks); }
    std::vector<double> MinorTicks(const std::vector<double>& majors) const {
        return GaugeScale::MinorTicks(majors, m_minorTicks);
    }

    GaugeScale m_scale;
    double m_value = 0.0;
    double m_shown = 0.0;
    std::string m_unit;
    int m_decimals = -1;
    bool m_localeNumbers = true;
    int m_majorTicks = 0;
    int m_minorTicks = 5;
    std::vector<GaugeZone> m_zones;
    std::string m_zonesText;
    bool m_colorByZone = true;
    bool m_showZoneBands = true;
    bool m_hasSetPoint = false;
    double m_setPoint = 0.0;
    bool m_showValue = true;
    bool m_animate = true;
    std::string m_caption;

private:
    class PaintItem;
    std::shared_ptr<PaintItem> m_paint;
    std::string m_zoneName;

    void LayoutParts();
    void UpdateZone(bool notify);
};

} // namespace jwgui
