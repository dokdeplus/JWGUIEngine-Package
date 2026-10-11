#pragma once

#include "UIChartBase.h"

namespace jwgui {

enum class PieLabels {
    None,
    Percent,  // 42%
    Value,    // 1,250
    Name      // Chrome
};

// Shares of a whole: a pie, or a donut with the total in the middle. Scene
// type `PieChart` (Toolbox Extended › CHARTS). Slices come from the first
// visible series, named by the categories; slices are 2 px apart, a slice
// steps out under the pointer, and the legend shows / hides slices.
class JWGUI_API UIPieChart : public UIChartBase {
public:
    UIPieChart(float x, float y, float w, float h);

    // Donut hole, percent of the radius (0 = a pie, up to 85).
    void SetHole(double percent);
    double GetHole() const { return m_hole; }
    // Where the first slice starts, clock degrees (0 = 12 o'clock).
    void SetStartAngle(double degrees);
    void SetSliceLabels(PieLabels labels);
    void SetSliceLabelsName(const std::string& name);  // "None" / "Percent" / "Value" / "Name"
    // Text in the donut's middle; empty = the total.
    void SetCenterText(const std::string& text);
    void SetSliceVisible(int slice, bool visible);
    bool IsSliceVisible(int slice) const;

    // The slice's share now (0..1), or 0 when hidden or empty.
    double SliceShare(int slice) const;
    // A slice's angles in the last paint (clock degrees from 12 o'clock).
    bool GetSliceAngles(int slice, float& startDeg, float& sweepDeg) const;
    // The series the slices come from (-1 = none).
    int SliceSeries() const;

protected:
    void DrawChart(IRenderer* renderer, float x, float y, float w, float h) override;
    bool HitTest(float localX, float localY, int& series, int& index) const override;
    std::vector<LegendEntry> LegendEntries() const override;
    ChartLegend AutoLegend(float w, float h, size_t entries) const override;
    void OnLegendClicked(int id) override;
    ChartPointEventArgs PointArgs(int series, int index) const override;

private:
    struct Slice {
        int index = -1;
        float start = 0.0f;  // clock degrees
        float sweep = 0.0f;
    };

    double m_hole = 0.0;
    double m_startAngle = 0.0;
    PieLabels m_labels = PieLabels::Percent;
    std::string m_centerText;
    std::vector<bool> m_hidden;  // by slice
    std::vector<Slice> m_slices;
    float m_cx = 0.0f, m_cy = 0.0f, m_radius = 0.0f, m_inner = 0.0f;

    size_t SliceCount() const;
    double Total() const;
};

} // namespace jwgui
