#pragma once

#include "Export.h"
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

// How one region writes dates, times and numbers. Patterns use the Windows
// notation (what GetLocaleInfoEx returns): d dd day, ddd dddd weekday name,
// M MM month, MMM MMMM month name, yy yyyy year, h hh 12-hour, H HH 24-hour,
// m mm minute, s ss second, t tt AM / PM, 'text' literal ('' = a quote).
// Anything else is literal too.
struct JWGUI_API UILocaleData {
    std::string name;                  // "ko-KR"
    std::string am = "AM";             // "오전"
    std::string pm = "PM";             // "오후"
    std::string shortTime;             // the region's clock: "tt h:mm", "h:mm tt", "HH:mm"
    std::string longTime;              // with seconds; empty = shortTime plus seconds
    std::string shortTime12;           // a 12-hour pattern where the usual one is 24-hour (says the mark's side); may be empty
    std::string shortDate;             // "yyyy-MM-dd", "M/d/yyyy", "dd.MM.yyyy"
    std::string weekdayDate;           // the date with its weekday; empty = shortDate and the weekday (weekdayFirst)
    std::string monthDay;              // month name or number and day: "M'월' d'일'", "MMM d"
    std::string yearMonth;             // a calendar's title: "yyyy'년' M'월'", "MMMM yyyy"
    std::array<std::string, 7> weekdayMin;    // Sunday first: "Su" / "일"; empty = from weekdayShort
    std::array<std::string, 7> weekdayShort;  // "Sun" / "일"
    std::array<std::string, 7> weekdayLong;   // "Sunday" / "일요일"
    std::array<std::string, 12> monthShort;   // "Jan" / "1월"
    std::array<std::string, 12> monthLong;    // "January" / "1월"
    int firstDayOfWeek = -1;           // 0 = Sunday, 1 = Monday, 6 = Saturday; -1 = from the region
    bool weekdayFirst = true;          // "Sun, 10/4" (en) vs "10. 4. (일)" (ko)
    std::string decimal = ".";
    std::string group = ",";

    // Filled by UILocale::Complete from the patterns.
    bool hour12 = false;               // the region's clock is 12-hour
    bool meridianFirst = false;        // AM / PM before the time ("오후 3:05")

    // The time pattern for a clock: hourMode 0 = the region's, 12 or 24 =
    // that clock with this region's separators and AM / PM side.
    std::string TimePattern(bool seconds, int hourMode = 0) const;
    // `datePattern` with the short weekday on this region's side:
    // "ddd, M/d" (weekdayFirst) or "M'월' d'일' (ddd)".
    std::string WithWeekday(const std::string& datePattern) const;
};

// One piece of a pattern.
struct JWGUI_API LocalePatternToken {
    enum class Kind { Literal, Year, Month, Day, Weekday, Hour12, Hour24, Minute, Second, Meridian };
    Kind kind = Kind::Literal;
    int width = 0;      // letters in the pattern: "MMM" = 3
    std::string text;   // Literal
};

// A date and time to format. weekday -1 = computed from the date.
struct JWGUI_API LocaleDateTime {
    int year = 2000, month = 1, day = 1;
    int hour = 0, minute = 0, second = 0;
    int weekday = -1;  // 0 = Sunday
};

// Process-wide format locale (dates, times, numbers). UI thread only. The UI
// language (words) is UIStrings'.
class JWGUI_API UILocale {
public:
    // --- Format locale ---
    // The locale controls use: a name ("ja-JP"), or empty for the system's
    // (the OS region settings, the browser's language). Repaints.
    static void SetLocale(const std::string& name);
    static const std::string& GetLocale();
    // The record for GetLocale() now.
    static std::shared_ptr<const UILocaleData> Current();
    // The record for a name ("" = the system's); built once, then cached.
    static std::shared_ptr<const UILocaleData> Data(const std::string& name);
    // The system locale's name; "en-US" without a platform.
    static std::string SystemName();
    // Goes up whenever the records may have changed (SetLocale, SystemChanged):
    // views that keep formatted pieces compare it and rebuild.
    static unsigned int Generation();

    // Where system data comes from; Application sets its host's
    // (IPlatformHost::SystemLocaleName / QueryLocale). Empty = built-ins only.
    struct Source {
        std::function<std::string()> systemName;
        std::function<bool(const std::string& name, UILocaleData& data)> query;
    };
    static void SetSource(Source source);
    // The OS settings changed: drops the cache and repaints.
    static void SystemChanged();

    // The built-in record for the name's language: "ko..." -> ko-KR, anything
    // else -> en-US. False when the language has no built-in (out = en-US).
    static bool BuiltIn(const std::string& name, UILocaleData& out);
    // Fills what a platform left out (shortest weekday names, weekdayDate,
    // longTime, the first day of the week from the region) and the derived
    // fields (hour12, meridianFirst).
    static void Complete(UILocaleData& data);
    // Reads the browser's report (WebPlatformHost): lines "key=value", where
    // a value is a list of Intl.formatToParts pieces "type\x1Fvalue" joined
    // by \x1E, or a list of names. Turns the pieces into patterns.
    static bool ReadIntlReport(const std::string& report, UILocaleData& data);

    // Short AM / PM and Sunday-based weekday names of Current().
    static std::string AmPm(bool pm);
    static std::string WeekdayAbbrev(int sunday0);

    // --- Patterns ---
    static std::vector<LocalePatternToken> ParsePattern(const std::string& pattern);
    static std::string JoinPattern(const std::vector<LocalePatternToken>& tokens);
    static std::string Format(const std::string& pattern, const LocaleDateTime& value, const UILocaleData& data);
    // The pattern as a 24-hour clock (no AM / PM) or a 12-hour one with the
    // mark before or after.
    static std::string To24Hour(const std::string& pattern);
    static std::string To12Hour(const std::string& pattern, bool meridianFirst);
    // Adds seconds after the minutes, with the separator before the minutes.
    static std::string AddSeconds(const std::string& pattern);

    // --- Numbers ---
    // `decimals` places, the locale's decimal mark; grouping adds its group
    // separator every three digits. Digits stay 0-9.
    static std::string FormatNumber(double value, int decimals, bool grouping, const UILocaleData& data);
    static std::string FormatNumber(double value, int decimals, bool grouping = false);
};

} // namespace jwgui
