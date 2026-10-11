#pragma once

#include "UIDateTimeBar.h"
#include "../Core/Event.h"
#include <memory>

namespace jwgui {

class UIPopup;
class UICalendar;

class JWGUI_API UIDateControl : public UIDateTimeBar {
public:
    UIDateControl(float x, float y, float w, float h);
    ~UIDateControl() override;

    void SetDate(int year, int month, int day);
    int GetYear() const { return m_year; }
    int GetMonth() const { return m_month; }
    int GetDay() const { return m_day; }

    void SetDateFormat(DateFormat format);
    void SetDateFormat(const std::string& text);
    DateFormat GetDateFormat() const { return m_format; }
    const char* GetDateFormatString() const;

    std::string GetDateString() const;
    bool SetDateString(const std::string& iso);

    EventHandler<std::string> DateChangedEvent;

    void OnGlobalMouseDown(float x, float y) override;

protected:
    std::string FieldText(const DateTimeSegment& seg) const override;
    void StepActive(int dir) override;
    void ApplyTypedNumber(DateTimeField field, int value) override;
    bool ApplyTypedMonth(int month) override;
    void ApplyAmPm(bool pm) override;
    void NotifyChanged() override;

    bool AllowsCalendar() const override { return true; }
    void OnCalendarClicked() override;
    void OnAccessoryChanged() override;
    bool HitCalendarPopup(float localX, float localY) const override;
    void RebuildSegments() override;

private:
    int m_year = 2000;
    int m_month = 1;
    int m_day = 1;
    DateFormat m_format = DateFormat::Locale;

    bool m_isDroppedDown = false;
    std::shared_ptr<UIPopup> m_popupView;
    std::shared_ptr<UICalendar> m_calendar;

    bool Assign(int year, int month, int day, bool notify);
    void ToggleCalendarPopup();
    void CloseCalendarPopup();
    void SyncPopupCalendar();
    bool MapLocalToScreen(float localX, float localY, float& screenX, float& screenY) const;
};

} // namespace jwgui
