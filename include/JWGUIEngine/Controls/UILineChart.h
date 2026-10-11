#pragma once

#include "UIChartBase.h"

namespace jwgui {

enum class LineCurve {
    Straight,
    Smooth  // a curve through the points
};

enum class LineMarkers {
    Auto,    // dots while the points are few and far apart
    Always,
    Never
};

// Lines over categories (Jan, Feb, ...) or over numbers / time on the x axis
// (series written as x:y points, or Append). Scene type `LineChart` (Toolbox
// Extended › CHARTS). 2 px lines, ringed markers, an optional 10% area wash;
// a crosshair snaps to the nearest x and one tooltip lists every series there.
// For live data, Append points and keep a window with SetMaxPoints.
class JWGUI_API UILineChart : public UIChartBase {
public:
    UILineChart(float x, float y, float w, float h);

    void SetCurve(LineCurve curve);
    void SetCurveName(const std::string& name);  // "Straight" / "Smooth"
    void SetMarkers(LineMarkers markers);
    void SetMarkersName(const std::string& name);  // "Auto" / "Always" / "Never"
    // A faint fill under each line.
    void SetArea(bool area);
    bool GetArea() const { return m_area; }
    // The y axis starts at zero.
    void SetFromZero(bool fromZero);

    // Live data: adds a point to a series (made if missing). Without x the
    // point goes one after the last. Points past SetMaxPoints drop off the
    // front, so the chart shows a moving window. No glide for appended points.
    void Append(int series, double x, double y);
    void Append(int series, double y);
    // Points kept per series (0 = all).
    void SetMaxPoints(int count);
    int GetMaxPoints() const { return m_maxPoints; }

    // The x axis is numeric (some series has x values).
    bool NumericX() const;
    // The point (of the longest series) the crosshair stood on in the last paint (-1 = none).
    int CrosshairIndex() const { return m_crossIndex; }
    const ChartAxis& GetValueAxis() const { return m_axis; }

protected:
    void DrawChart(IRenderer* renderer, float x, float y, float w, float h) override;
    bool HitTest(float localX, float localY, int& series, int& index) const override;
    std::vector<LegendEntry> LegendEntries() const override;
    ChartPointEventArgs PointArgs(int series, int index) const override;

private:
    LineCurve m_curve = LineCurve::Straight;
    LineMarkers m_markers = LineMarkers::Auto;
    bool m_area = false;
    bool m_fromZero = false;
    int m_maxPoints = 0;
    ChartAxis m_axis;
    UIRect m_plot;
    double m_xMin = 0.0, m_xMax = 1.0;  // the x range drawn
    int m_crossIndex = -1;

    double PointX(const ChartSeries& series, size_t i) const;
    float ToScreenX(double x) const;
    size_t PointCount() const;
    // The x position (data) nearest to a screen x, as an index into the
    // reference series (the longest visible one).
    int NearestIndex(float screenX) const;
    int ReferenceSeries() const;
};

} // namespace jwgui
