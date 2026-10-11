#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include "DateTimeSegments.h"
#include <string>
#include <vector>

namespace jwgui {

// The weekday in the first column.
enum class CalendarWeekStart {
    Locale,   // the region's (UILocale firstDayOfWeek)
    Sunday,
    Monday,
    Saturday
};

class JWGUI_API UICalendar : public UIRenderView {
public:
    UICalendar(float x, float y, float w, float h);
    ~UICalendar() override = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void SetDate(int year, int month, int day);
    int GetYear() const { return m_year; }
    int GetMonth() const { return m_month; }
    int GetDay() const { return m_day; }

    std::string GetDateString() const;
    bool SetDateString(const std::string& iso);

    void SetDateFormat(DateFormat format);
    void SetDateFormat(const std::string& text);
    DateFormat GetDateFormat() const { return m_format; }
    const char* GetDateFormatString() const;

    void SetWeekStart(CalendarWeekStart start);
    CalendarWeekStart GetWeekStart() const { return m_weekStart; }
    void SetWeekStartName(const std::string& name);  // "Locale" / "Sunday" / "Monday" / "Saturday"
    // 0 = Sunday ... 6 = Saturday, the setting or the locale's.
    int FirstWeekday() const;

    EventHandler<std::string> DateChangedEvent;
    EventHandler<std::string> DateActivatedEvent;

    void OnStyleChanged() override;
    void RenderContent(IRenderer* renderer, float absX, float absY) override;
    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;

    static constexpr float kPreferredWidth = 224.0f;
    static constexpr float kPreferredHeight = 248.0f;

private:
    struct Cell {
        int year = 2000;
        int month = 1;
        int day = 1;
        bool inMonth = false;
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    int m_year = 2000;
    int m_month = 1;
    int m_day = 1;
    int m_viewYear = 2000;
    int m_viewMonth = 1;
    DateFormat m_format = DateFormat::Locale;
    CalendarWeekStart m_weekStart = CalendarWeekStart::Locale;
    int m_todayY = 2000;
    int m_todayM = 1;
    int m_todayD = 1;

    uint32_t m_cachedFgColor = 0xFFFFFFFF;
    uint32_t m_cachedMutedColor = 0xFF808080;
    uint32_t m_cachedOutlineColor = 0xFF454545;
    uint32_t m_cachedArrowColor = 0xFFFFFFFF;
    uint32_t m_cachedSelBgColor = 0xFF264F78;
    uint32_t m_cachedHoverBgColor = 0xFF3E3E42;
    float m_cachedOutlineThick = 1.0f;
    float m_cachedFontSize = 14.0f;
    std::string m_cachedFontFamily = UIStyle::DefaultFontFamily();
    UIBackground m_cachedBg;
    Thickness m_cachedPadding = Thickness(6.0f);

    float m_headerH = 32.0f;
    float m_weekH = 22.0f;
    UIRect m_prevRect;
    UIRect m_nextRect;
    std::vector<Cell> m_cells;

    void BuildGrid(float innerW, float innerH);
    void LayoutNavRects();
    bool Assign(int year, int month, int day, bool notify, bool activate);
    void MoveView(int monthDelta);
    void ActivateCell(const Cell& cell);
};

} // namespace jwgui
