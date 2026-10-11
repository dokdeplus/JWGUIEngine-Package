#pragma once

#include "../Core/Export.h"
#include <string>
#include <vector>

namespace jwgui {

// One series of a chart. Category charts (bar, pie, a line by category) use
// `values`, one per category; a line chart with numbers on the x axis also
// has `xs`, one per value.
struct JWGUI_API ChartSeries {
    std::string name;
    std::vector<double> values;
    std::vector<double> xs;   // empty = by category (index)
    std::string color;        // "#RRGGBB" or a zone name; empty = the series slot color
    bool visible = true;
};

// The scene's `series` text: "Sales: 12, 18, 9 | Cost: 8, 10, 7". A series
// without a name is called "Series <n>"; "0:20.5, 10:21" are x:y points.
// Numbers are written with '.', whatever the locale. Unreadable items are skipped.
JWGUI_API std::vector<ChartSeries> ParseChartSeries(const std::string& text);
JWGUI_API std::string FormatChartSeries(const std::vector<ChartSeries>& series);
// The scene's `labels` text: "Mon, Tue, Wed".
JWGUI_API std::vector<std::string> ParseChartLabels(const std::string& text);

// A reference line: "80:warn; 95:#E24B4A" (a zone name or a color).
struct JWGUI_API ChartThreshold {
    double value = 0.0;
    std::string color;
};
JWGUI_API std::vector<ChartThreshold> ParseChartThresholds(const std::string& text);

// A value axis with round ticks (1-2-5 steps) covering [lo, hi].
struct JWGUI_API ChartAxis {
    double min = 0.0;
    double max = 1.0;
    double step = 1.0;
    int decimals = 0;
    std::vector<double> ticks;
    double ToT(double value) const { return max > min ? (value - min) / (max - min) : 0.0; }
};
// includeZero: bars always start at zero. targetTicks: about how many steps.
JWGUI_API ChartAxis NiceChartAxis(double lo, double hi, int targetTicks, bool includeZero);
// Round ticks inside [lo, hi] without widening it (a moving time window).
JWGUI_API ChartAxis InnerChartAxis(double lo, double hi, int targetTicks);

} // namespace jwgui
