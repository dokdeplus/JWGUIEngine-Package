#pragma once

#include "UIGaugeBase.h"

namespace jwgui {

enum class DigitalDisplayMode {
    SevenSegment,  // drawn LED segments with a decimal point dot
    Text           // the theme font, the locale's decimal mark
};

// A numeric readout like a panel meter: a fixed number of digit cells, the
// value right-aligned, a unit after it. Scene type `DigitalDisplay`
// (Toolbox Extended › INDICATORS). Range, value, decimals, unit, zones and
// ZoneChangedEvent come from UIGaugeBase; a value in a zone other than
// normal / ok takes the zone's color.
//
// Style parts (Type.part): DigitalDisplay (the window: Background, outline),
// .segment (lit, ForegroundColor), .segment-off (unlit; default the lit
// color, faint), .text (Text mode digits; FontFamily), .unit, .caption.
class JWGUI_API UIDigitalDisplay : public UIGaugeBase {
public:
    UIDigitalDisplay(float x, float y, float w, float h);

    // Digit cells, the minus sign included (1..12).
    void SetDigits(int digits);
    int GetDigits() const { return m_digits; }
    // 0042.5 instead of blanks.
    void SetLeadingZeros(bool zeros);
    bool GetLeadingZeros() const { return m_leadingZeros; }
    void SetDisplay(DigitalDisplayMode mode);
    DigitalDisplayMode GetDisplay() const { return m_display; }
    void SetDisplayName(const std::string& name);  // "SevenSegment" / "Text"
    // Seven segment: unlit segments drawn faintly.
    void SetShowUnlit(bool show);
    bool GetShowUnlit() const { return m_showUnlit; }

    // The cells as shown, a '.' after the digit that carries the point:
    // "   12.5", "-0042.5"; dashes when the value does not fit ("-----").
    std::string CellText(double value) const;
    // The digit color now: the zone's when the value is in one (not normal / ok).
    uint32_t DigitColor() const;

protected:
    void DrawGauge(IRenderer* renderer, float x, float y, float w, float h) override;

private:
    int m_digits = 5;
    bool m_leadingZeros = false;
    DigitalDisplayMode m_display = DigitalDisplayMode::SevenSegment;
    bool m_showUnlit = true;

    uint32_t OnlyPartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
};

} // namespace jwgui
