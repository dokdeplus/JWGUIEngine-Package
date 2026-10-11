#pragma once

#include "UIDateTimeBar.h"
#include "../Core/Event.h"

namespace jwgui {

class JWGUI_API UITimeControl : public UIDateTimeBar {
public:
    UITimeControl(float x, float y, float w, float h);
    ~UITimeControl() override = default;

    void SetTime(int hour, int minute, int second);
    int GetHour() const { return m_hour; }
    int GetMinute() const { return m_minute; }
    int GetSecond() const { return m_second; }

    void SetTimeFormat(TimeFormat format);
    void SetTimeFormat(const std::string& text);
    TimeFormat GetTimeFormat() const { return m_format; }
    const char* GetTimeFormatString() const;

    std::string GetTimeString() const;
    bool SetTimeString(const std::string& iso);

    EventHandler<std::string> TimeChangedEvent;

protected:
    std::string FieldText(const DateTimeSegment& seg) const override;
    void StepActive(int dir) override;
    void ApplyTypedNumber(DateTimeField field, int value) override;
    bool ApplyTypedMonth(int month) override;
    void ApplyAmPm(bool pm) override;
    void NotifyChanged() override;
    void RebuildSegments() override;

private:
    int m_hour = 0;
    int m_minute = 0;
    int m_second = 0;
    TimeFormat m_format = TimeFormat::Locale;

    bool Assign(int hour, int minute, int second, bool notify);
    bool IsTwelveHourFormat() const;
};

} // namespace jwgui
