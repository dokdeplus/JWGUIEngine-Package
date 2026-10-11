#pragma once

#include "UIChartBase.h"

namespace jwgui {

enum class BarOrientation {
    Vertical,   // columns rising from the bottom
    Horizontal  // bars from the left, categories down the side (long names)
};

enum class BarMode {
    Grouped,  // the series side by side in each category
    Stacked   // the series on top of each other (part of a whole)
};

// Bars per category for one or more series. Scene type `BarChart` (Toolbox
// Extended › CHARTS). Bars start at zero, at most maxBarWidth thick with a
// rounded end, 2 px apart; each bar is its own hover target with a tooltip.
class JWGUI_API UIBarChart : public UIChartBase {
public:
    UIBarChart(float x, float y, float w, float h);

    void SetOrientation(BarOrientation orientation);
    BarOrientation GetOrientation() const { return m_orientation; }
    void SetOrientationName(const std::string& name);  // "Vertical" / "Horizontal"
    void SetMode(BarMode mode);
    BarMode GetMode() const { return m_mode; }
    void SetModeName(const std::string& name);  // "Grouped" / "Stacked"
    // Thickest a bar gets (px); a wide band keeps the rest as air.
    void SetMaxBarWidth(double px);

    // Where a bar was drawn in the last paint (absolute); false if it was not.
    bool GetBarRect(int series, int index, UIRect& out) const;
    const ChartAxis& GetValueAxis() const { return m_axis; }
    // Categories drawn: the labels, or the longest series.
    size_t CategoryCount() const;

protected:
    void DrawChart(IRenderer* renderer, float x, float y, float w, float h) override;
    bool HitTest(float localX, float localY, int& series, int& index) const override;

private:
    struct Bar {
        int series = -1;
        int index = -1;
        UIRect rect;  // painted
        UIRect hit;   // pointer target (bigger than the bar)
    };

    BarOrientation m_orientation = BarOrientation::Vertical;
    BarMode m_mode = BarMode::Grouped;
    float m_maxBarWidth = 24.0f;
    std::vector<Bar> m_bars;
    ChartAxis m_axis;
};

} // namespace jwgui
