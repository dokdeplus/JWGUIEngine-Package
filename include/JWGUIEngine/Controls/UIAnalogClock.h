#pragma once

#include "UIClockBase.h"
#include "PartImage.h"
#include <string>
#include <vector>

namespace jwgui {

enum class ClockNumerals {
    Arabic,
    Roman,
    None
};

// Hour and minute hands.
enum class ClockHandShape {
    Line,      // a round-capped line
    Tapered,   // wide at the center, narrow at the tip
    Baton,     // a flat bar with square ends
    Arrow,     // a thin shaft with an arrow head
    Leaf,      // widest a third of the way out, pointed at both ends
    Skeleton,  // a tapered outline, open inside
};

enum class ClockSecondShape {
    Line,      // a thin line with a tail
    Lollipop,  // a line with a disc near the tip (railway station clocks)
    Arc,       // no hand: an arc along the rim from 12 o'clock to the second
};

enum class ClockMarks {
    Lines,  // hour strokes and finer minute strokes
    Dots,
    Bars,   // thick hour bars, thin minute strokes
    None,
};

enum class ClockHand { Hour, Minute, Second };

// Everything that shapes the face and hands (colors are style). Lengths and
// widths are percent of the face's radius.
struct ClockLook {
    ClockHandShape handShape = ClockHandShape::Line;
    ClockSecondShape secondShape = ClockSecondShape::Line;
    float hourLength = 50.0f;
    float minuteLength = 74.0f;
    float secondLength = 83.0f;
    float hourWidth = 5.5f;
    float minuteWidth = 3.5f;
    float secondWidth = 1.3f;  // Arc: the arc's thickness
    bool handTail = true;
    ClockMarks marks = ClockMarks::Lines;
    bool minuteMarks = true;
    float markLength = 11.0f;  // hour marks; minute marks are shorter
    ClockNumerals numerals = ClockNumerals::Arabic;
    float numeralSize = 0.0f;  // 0 = by the numeral style
    bool innerRing = false;    // a thin circle inside the marks
    float centerCap = 5.0f;
};

// Round wall clock with hands. Scene type `AnalogClock` (Toolbox
// Extended › DATE & TIME).
//
// Look (ControlsPlan §8): a preset (Classic, Modern, Minimal, Station,
// Dashboard) fills a ClockLook; every field can then be changed. A subclass
// can replace one drawing step (DrawFace, DrawMarks, DrawNumerals, DrawTexts,
// DrawHand, DrawCenter) and keep the rest.
//
// Style parts (Type.part): AnalogClock (box, bare by default), .face
// (Background solid or gradient, OutlineColor / OutlineThickness rim), .tick,
// .hand, .hour-hand, .minute-hand (fall back to .hand), .second
// (ForegroundColor), .text (numerals; FontFamily), .caption (caption and date).
class JWGUI_API UIAnalogClock : public UIClockBase {
public:
    UIAnalogClock(float x, float y, float w, float h);

    void SetNumerals(ClockNumerals numerals);
    ClockNumerals GetNumerals() const { return m_look.numerals; }
    void SetNumeralsName(const std::string& name);  // "Arabic" / "Roman" / "None"

    // The second hand sweeps (redraws ~20 times a second) instead of stepping.
    void SetSmoothSeconds(bool smooth);
    bool GetSmoothSeconds() const { return m_smoothSeconds; }

    void SetLook(const ClockLook& look);
    const ClockLook& GetLook() const { return m_look; }
    // "Classic", "Modern", "Minimal", "Station", "Dashboard"; another name = Classic.
    static ClockLook Preset(const std::string& name);
    static const std::vector<std::string>& PresetNames();
    void SetPreset(const std::string& name);
    const std::string& GetPreset() const { return m_preset; }

    // One field at a time (the scene's properties).
    void SetHandShapeName(const std::string& name);
    void SetSecondShapeName(const std::string& name);
    void SetMarksName(const std::string& name);
    void SetHourLength(float percent);
    void SetMinuteLength(float percent);
    void SetSecondLength(float percent);
    void SetHourWidth(float percent);
    void SetMinuteWidth(float percent);
    void SetSecondWidth(float percent);
    void SetHandTail(bool tail);
    void SetMinuteMarks(bool show);
    void SetMarkLength(float percent);
    void SetNumeralSize(float percent);
    void SetInnerRing(bool show);
    void SetCenterCap(float percent);

    // Pictures (Project resource ids) instead of the drawn face and hands.
    // A hand picture stands pointing at 12 o'clock; HandPivot is its point
    // on the center ("0.5,0.85" = middle, near the bottom), and the part above
    // the pivot is the hand's length. Empty = drawn.
    void SetFaceImage(const std::string& id);
    void SetHourHandImage(const std::string& id);
    void SetMinuteHandImage(const std::string& id);
    void SetSecondHandImage(const std::string& id);
    void SetHandPivot(const std::string& pivot);

    static const char* HandShapeName(ClockHandShape shape);
    static const char* SecondShapeName(ClockSecondShape shape);
    static const char* MarksName(ClockMarks marks);
    static const char* NumeralsName(ClockNumerals numerals);

    // .hour-hand / .minute-hand fall back to .hand.
    bool ResolvePartColor(const std::string& part, UIStylePropertyID prop, uint32_t& out) const override;

protected:
    void DrawClock(IRenderer* renderer, float x, float y, float w, float h, const Time& time) override;
    unsigned int TickInterval(bool& alignToWall) const override;

    // The drawing steps, in this order. (cx, cy) is the center and `radius`
    // the face's radius.
    virtual void DrawFace(IRenderer* renderer, float cx, float cy, float radius);
    virtual void DrawMarks(IRenderer* renderer, float cx, float cy, float radius);
    virtual void DrawNumerals(IRenderer* renderer, float cx, float cy, float radius);
    // The caption (upper half) and the date (lower half).
    virtual void DrawTexts(IRenderer* renderer, float cx, float cy, float radius, const Time& time);
    // `degrees`: clockwise from 12 o'clock. Hour, minute, then the center
    // cap, then the second hand (when seconds show).
    virtual void DrawHand(IRenderer* renderer, ClockHand hand, float degrees, float cx, float cy, float radius);
    virtual void DrawCenter(IRenderer* renderer, float cx, float cy, float radius);

    // Where the marks end, inside the rim.
    float MarksOuter(float radius) const;
    // Where the hour marks begin (toward the center).
    float MarksInner(float radius) const;
    uint32_t HandColor(ClockHand hand) const;

    ClockLook m_look;
    std::string m_preset = "Classic";
    bool m_smoothSeconds = false;
    PartImage m_faceImage;
    PartImage m_handImages[3];  // ClockHand order
    float m_pivotX = 0.5f;
    float m_pivotY = 0.85f;
};

} // namespace jwgui
