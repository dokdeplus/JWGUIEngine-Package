#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/TimerQueue.h"
#include <string>

namespace jwgui {

// What both clocks share: where the time comes from (now in local time, now
// at a UTC offset, or a fixed time), the repaint tick, the caption, seconds
// and the date line, and the style-part lookups. UIAnalogClock and
// UIDigitalClock only draw (DrawClock) and say how often they need a repaint
// (TickInterval).
class JWGUI_API UIClockBase : public UIRenderView {
public:
    UIClockBase(float x, float y, float w, float h);
    ~UIClockBase() override;

    void OnCreate() override;
    void OnLayout() override;
    void BuildRenderItems() override;

    void SetShowSeconds(bool show);
    bool GetShowSeconds() const { return m_showSeconds; }
    void SetShowDate(bool show);
    bool GetShowDate() const { return m_showDate; }

    // "HH:MM:SS" (or "HH:MM") shows that time and stops ticking; empty = now.
    void SetFixedTime(const std::string& time);
    const std::string& GetFixedTime() const { return m_fixedTime; }
    // Empty = local time. "UTC" / "Z", "+09:00", "-5", "+5:30": UTC plus that offset.
    void SetUtcOffset(const std::string& offset);
    const std::string& GetUtcOffset() const { return m_utcOffset; }

    void SetCaption(const std::string& caption);
    const std::string& GetCaption() const { return m_caption; }

    // The time shown now (fixed or current).
    struct Time {
        int year = 2000, month = 1, day = 1;
        int hour = 0, minute = 0, second = 0, millis = 0;
        int weekday = 6;  // 0 = Sunday
    };
    Time CurrentTime() const;
    // Minutes east of UTC for an offset string; false for "" or unreadable text.
    static bool ParseUtcOffset(const std::string& text, int& outMinutes);

protected:
    // Draws the clock inside (x, y, w, h) for that time.
    virtual void DrawClock(IRenderer* renderer, float x, float y, float w, float h, const Time& time) = 0;
    // How often the face changes: ms between repaints, and whether they
    // fall on the wall clock's boundaries (seconds, minutes).
    virtual unsigned int TickInterval(bool& alignToWall) const;
    // Settings changed the tick (call from setters that affect TickInterval).
    void UpdateTicking();

    // "Type.part" lookups (with the base types of an app subclass).
    uint32_t PartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    // A color set for `part` by name (the control's own .part, #id.part,
    // Type.part, .part), else `fallback`: for a part that falls back to another
    // part's color (DigitalClock.hours -> .segment).
    uint32_t NamedPartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    float PartFloat(const char* part, UIStylePropertyID prop, float fallback) const;
    std::string FontFamily() const;

    bool m_showSeconds = true;
    bool m_showDate = false;
    std::string m_fixedTime;
    std::string m_utcOffset;
    std::string m_caption;

private:
    class PaintItem;
    std::shared_ptr<PaintItem> m_paint;
    TimerId m_timer = 0;

    void LayoutParts();
    void StopTicking();
};

} // namespace jwgui
