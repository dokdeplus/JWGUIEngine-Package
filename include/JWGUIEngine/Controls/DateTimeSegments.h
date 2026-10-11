#pragma once

#include "../Core/Export.h"
#include <string>
#include <vector>

namespace jwgui {

// Locale = the region's short date (UILocale); the rest are fixed orders.
// Order matters: the editor lists them in this order.
enum class DateFormat {
    Locale,
    YMD_Dash,
    YMD_Slash,
    YMD_Dot,
    DMY_Slash,
    DMY_Dash,
    MDY_Slash,
    MDY_Abbrev,
    MDY_Full
};

// Locale = the region's clock with seconds, LocaleShort without them.
enum class TimeFormat {
    Locale,
    LocaleShort,
    Hms24,
    Hm24,
    Hms12,
    Hm12
};

enum class DateTimeField {
    Year,
    Month,
    Day,
    Hour,
    Minute,
    Second,
    AmPm,
    Literal
};

enum class MonthNameStyle {
    Numeric,
    Abbrev,
    Full
};

enum class DateTimeAccessory {
    None,
    Spin,
    Calendar
};

struct DateTimeSegment {
    DateTimeField field = DateTimeField::Literal;
    std::string literal;
    int digits = 0;         // typed digits (and the zero padding when pad)
    bool pad = true;        // "04" (pattern MM / dd / HH) or "4" (M / d / H)
    MonthNameStyle monthStyle = MonthNameStyle::Numeric;
};

// A format's pattern in UILocale's notation ("yyyy-MM-dd", "hh:mm tt");
// Locale formats read the current locale.
JWGUI_API std::string DateFormatPattern(DateFormat format);
JWGUI_API std::string TimeFormatPattern(TimeFormat format);
// Editable pieces of a pattern: the weekday is left out, a two-digit year
// is edited as four digits.
JWGUI_API std::vector<DateTimeSegment> BuildPatternSegments(const std::string& pattern);
// The pattern shows a 12-hour clock.
JWGUI_API bool PatternIsTwelveHour(const std::string& pattern);
std::vector<DateTimeSegment> BuildDateSegments(DateFormat format);
std::vector<DateTimeSegment> BuildTimeSegments(TimeFormat format);

JWGUI_API const char* DateFormatToString(DateFormat format);
JWGUI_API bool DateFormatFromString(const std::string& text, DateFormat& out);
JWGUI_API const char* TimeFormatToString(TimeFormat format);
JWGUI_API bool TimeFormatFromString(const std::string& text, TimeFormat& out);
JWGUI_API const char* DateTimeAccessoryToString(DateTimeAccessory accessory);
JWGUI_API bool DateTimeAccessoryFromString(const std::string& text, DateTimeAccessory& out);
JWGUI_API std::string FormatMonthTitle(DateFormat format, int year, int month);

bool IsLeapYear(int year);
int DaysInMonth(int year, int month);
JWGUI_API void ClampDate(int& year, int& month, int& day);
JWGUI_API void ClampTime(int& hour, int& minute, int& second);

void StepDateField(DateTimeField field, int dir, int& year, int& month, int& day);
void StepTimeField(DateTimeField field, int dir, int& hour24, int& minute, int& second, bool twelveHour);
void AddDays(int& year, int& month, int& day, int delta);
void AddMonths(int& year, int& month, int& day, int delta);
int WeekdaySunday0(int year, int month, int day);

int DisplayHour12(int hour24);
int Hour24From12(int hour12, bool pm);

const char* MonthAbbrev(int month); // always English
const char* MonthFull(int month);   // always English
// The current locale's month name (MonthNameStyle Abbrev / Full).
JWGUI_API std::string MonthName(int month, MonthNameStyle style);
// The month whose name starts with `typed` (the locale's names, then
// English); 0 when none or more than one match.
JWGUI_API int MatchMonthName(const std::string& typed, MonthNameStyle style);

std::string PadDigits(int value, int width);
std::string FormatDateField(const DateTimeSegment& seg, int year, int month, int day);
std::string FormatTimeField(const DateTimeSegment& seg, int hour24, int minute, int second, bool twelveHour);

JWGUI_API void LocalNow(int& year, int& month, int& day, int& hour, int& minute, int& second);
JWGUI_API std::string FormatIsoDate(int year, int month, int day);
JWGUI_API bool ParseIsoDate(const std::string& text, int& year, int& month, int& day);
JWGUI_API std::string FormatIsoTime(int hour, int minute, int second);
JWGUI_API bool ParseIsoTime(const std::string& text, int& hour, int& minute, int& second);

} // namespace jwgui
