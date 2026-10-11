#pragma once

#include "UIClockBase.h"
#include "DateTimeSegments.h"

namespace jwgui {

enum class DigitalClockDisplay {
    Text,          // the theme font; digits in equal cells so they do not shift
    SevenSegment   // drawn LED / LCD segments, slanted, with dim unlit segments
};

// 12 or 24 hours.
enum class ClockHours {
    Locale,   // the region's clock (UILocale: ko-KR and en-US 12-hour, de-DE 24-hour)
    Hour12,
    Hour24
};

// How a 12-hour clock marks morning and afternoon.
enum class ClockMeridian {
    Locale,   // the locale's words and side (ko-KR: 오후 3:05, en-US: 3:05 PM)
    English,  // AM / PM after the time; drawn in segments on a SevenSegment display
    Custom,   // amText / pmText, before or after the time (SetMeridianBefore)
    None      // no mark
};

// HH:MM[:SS] clock. Scene type `DigitalClock` (Toolbox Extended › DATE & TIME).
// The time fills the box; a caption sits above it and the date below.
//
// Style parts (Type.part): DigitalClock (box, bare by default), .text (time in
// Text mode; FontFamily), .caption (caption, date in Text mode), .segment (lit
// segments, ForegroundColor), .segment-off (unlit segments; default: each
// unit's color, faint). Each unit has its own part that falls back to the
// display's color: .hours, .minutes, .seconds, .colon, .meridian, .date.
class JWGUI_API UIDigitalClock : public UIClockBase {
public:
    UIDigitalClock(float x, float y, float w, float h);

    void SetDisplay(DigitalClockDisplay display);
    DigitalClockDisplay GetDisplay() const { return m_display; }
    void SetDisplayName(const std::string& name);  // "Text" / "SevenSegment"

    // The region's clock, 12-hour with a morning / afternoon mark
    // (SetMeridian), or 24-hour.
    void SetHours(ClockHours hours);
    ClockHours GetHours() const { return m_hours; }
    void SetHoursName(const std::string& name);  // "Locale" / "12-hour" / "24-hour"
    void SetHour24(bool hour24);                 // 24-hour, or 12-hour
    // The clock shows 24 hours now (Locale: the region's).
    bool IsHour24() const;
    // 12-hour mark (unused on a 24-hour clock).
    void SetMeridian(ClockMeridian meridian);
    ClockMeridian GetMeridian() const { return m_meridian; }
    void SetMeridianName(const std::string& name);  // "Locale" / "English" / "Custom" / "None"
    // Custom mark words and side.
    void SetAmText(const std::string& text);
    void SetPmText(const std::string& text);
    void SetMeridianBefore(bool before);
    void SetMeridianPositionName(const std::string& name);  // "Before" / "After"

    // The date line: Locale = the region's date with its weekday, or a fixed
    // order with the weekday on the region's side.
    void SetDateFormat(DateFormat format);
    DateFormat GetDateFormat() const { return m_dateFormat; }
    void SetDateFormatName(const std::string& name);  // DateFormatToString names

    // The colon goes dark every other second.
    void SetBlinkColon(bool blink);
    bool GetBlinkColon() const { return m_blinkColon; }
    // Seven segment: draw unlit segments faintly, like a real display.
    void SetShowUnlit(bool show);
    bool GetShowUnlit() const { return m_showUnlit; }

    // Width of a glyph at height 1 ('M' is wider, ':' narrower).
    static float SegmentGlyphWidth(char glyph);
    // Every character of text can be drawn as a segment glyph.
    static bool CanDrawAsSegments(const std::string& text);

    // "10:10:30" / "10:10 PM" / "오후 10:10" as the clock reads.
    std::string FormatTime(const Time& time) const;
    // The date line in Text mode ("Sun, 10/4/2026", "2026-10-04 (일)").
    std::string FormatDate(const Time& time) const;

    // The color a unit is drawn with now: "hours", "minutes", "seconds",
    // "colon", "meridian" or "date" take their own part's rule, else the
    // display's color (.segment; in Text mode .text, and .caption for the date).
    uint32_t UnitColor(const std::string& unit) const;
    // Unlit segments of a unit: .segment-off when set, else the unit's color, faint.
    uint32_t UnlitColor(uint32_t unitColor) const;
    // Units and .segment-off as drawn (the hours stand in for "every unit").
    bool ResolvePartColor(const std::string& part, UIStylePropertyID prop, uint32_t& out) const override;

    // One segment glyph in (x, y, w, h): digits, '-', ' ', and the letters a
    // seven-segment display can show (A b C d E F H L n o P r t U); 'M' adds
    // two diagonal bars, as on 14-segment displays. thickness is the segment
    // width, slant the x shift per unit of height above the middle. Public so
    // other controls (a digital display) can draw digits.
    static void DrawSegmentGlyph(IRenderer* renderer, char glyph, float x, float y, float w, float h,
                                 float thickness, float slant, uint32_t on, uint32_t off, bool showOff);

protected:
    void DrawClock(IRenderer* renderer, float x, float y, float w, float h, const Time& time) override;
    unsigned int TickInterval(bool& alignToWall) const override;

private:
    DigitalClockDisplay m_display = DigitalClockDisplay::Text;
    ClockHours m_hours = ClockHours::Locale;
    ClockMeridian m_meridian = ClockMeridian::Locale;
    std::string m_amText = "AM";
    std::string m_pmText = "PM";
    bool m_meridianBefore = false;
    DateFormat m_dateFormat = DateFormat::Locale;
    bool m_blinkColon = false;
    bool m_showUnlit = true;

    std::string Digits(const Time& time, bool padHour) const;
    std::string Meridian(const Time& time) const;
    // The mark comes before the digits ("오후 3:05").
    bool MeridianFirst() const;
    // The date as segment digits in the format's order: "10-04-2026".
    std::string SegmentDate(const Time& time) const;
    void DrawTextTime(IRenderer* r, float x, float top, float w, float boxH, const Time& t);
    void DrawSegmentTime(IRenderer* r, float x, float top, float w, float boxH, const Time& t);
};

} // namespace jwgui
