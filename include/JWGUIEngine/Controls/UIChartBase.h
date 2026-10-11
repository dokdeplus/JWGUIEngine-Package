#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include "ChartData.h"
#include <string>
#include <vector>

namespace jwgui {

// A data point the pointer is on or clicked: series -1 = every series at an
// x (a line chart's crosshair), index -1 = nothing.
struct ChartPointEventArgs : public EventArgs {
    int series = -1;
    int index = -1;
    double value = 0.0;
    std::string seriesName;
    std::string category;
};

enum class ChartLegend {
    Auto,    // bottom when there are two or more entries, none for one
    Top,
    Bottom,
    Right,
    None
};

// What BarChart, PieChart and LineChart share: the data (categories and
// series, from code or from the scene's `labels` / `series` text), the title,
// legend, value text in the locale, reference lines, the hover tooltip and
// events, and values that glide to new data.
//
// Style parts (Type.part, all ForegroundColor unless noted): Type (box, bare),
// .caption (title), .axis (zero line), .grid (gridlines), .label (axis and
// category text), .value (value labels), .legend, .tooltip (Background,
// OutlineColor, ForegroundColor), .marker (OutlineColor: the ring round line
// markers, the surface color). Series colors: Type.series-<n> / .series-<n>
// (n = 1..8, global classes like the zone colors); a 9th series is gray.
class JWGUI_API UIChartBase : public UIRenderView {
public:
    static constexpr int kSeriesColors = 8;

    UIChartBase(float x, float y, float w, float h);
    ~UIChartBase() override;

    void OnCreate() override;
    void OnLayout() override;
    void BuildRenderItems() override;
    bool OnMouseMove(float x, float y) override;
    void OnMouseLeave() override;
    bool OnMouseDown(float x, float y, MouseButton button) override;

    // --- Data ---
    void SetCategories(std::vector<std::string> categories);
    const std::vector<std::string>& GetCategories() const { return m_categories; }
    void SetLabelsText(const std::string& text);  // scene `labels`: "Mon, Tue, Wed"
    void SetSeries(std::vector<ChartSeries> series);
    const std::vector<ChartSeries>& GetSeries() const { return m_series; }
    void SetSeriesText(const std::string& text);  // scene `series`: "Sales: 12, 18 | Cost: 8, 10"
    int AddSeries(const std::string& name, std::vector<double> values = {});
    void SetSeriesValues(int series, std::vector<double> values);
    void SetSeriesColor(int series, const std::string& color);  // "#RRGGBB" / zone name / "" = slot
    void SetSeriesVisible(int series, bool visible);
    void ClearSeries();
    // The value drawn now (it glides from the old data for a moment).
    double ShownValue(int series, int index) const;

    // --- Look ---
    void SetCaption(const std::string& caption);
    const std::string& GetCaption() const { return m_caption; }
    void SetLegend(ChartLegend legend);
    ChartLegend GetLegend() const { return m_legend; }
    void SetLegendName(const std::string& name);  // "Auto" / "Top" / "Bottom" / "Right" / "None"
    // Values written at the bars' tips / slices / line ends.
    void SetShowValues(bool show);
    bool GetShowValues() const { return m_showValues; }
    // Places after the point in values and tooltips; -1 = as the data needs (up to 3).
    void SetDecimals(int decimals);
    void SetUnit(const std::string& unit);
    const std::string& GetUnit() const { return m_unit; }
    // The locale's decimal mark and digit groups (12,500.5), or Invariant (12500.5).
    void SetLocaleNumbers(bool locale);
    void SetNumbersName(const std::string& name);  // "Locale" / "Invariant"
    // Reference lines: "80:warn; 95:alarm" (zone names or #RRGGBB).
    void SetThresholds(const std::string& text);
    void SetAnimate(bool animate);

    // The color of a series now: its own, else .series-<n>, gray past eight.
    uint32_t SeriesColor(int series) const;
    // The n-th categorical color (0-based): .series-<n+1>, gray past eight.
    uint32_t SlotColor(int slot) const;
    std::string FormatNumber(double value, int decimals) const;
    // A value as the tooltip writes it, with the unit.
    std::string FormatValue(double value) const;

    // What the pointer is on (-1 = nothing).
    int HoverSeries() const { return m_hoverSeries; }
    int HoverIndex() const { return m_hoverIndex; }

    EventHandler<ChartPointEventArgs> PointHoveredEvent;
    EventHandler<ChartPointEventArgs> PointClickedEvent;

protected:
    struct LegendEntry {
        std::string name;
        uint32_t color = 0;
        bool hidden = false;
        bool lineKey = false;  // a short line (line charts) instead of a square
        int id = 0;            // series index, or slice index for a pie
        UIRect rect;           // where it was drawn (absolute), for clicks
    };
    struct TooltipRow {
        uint32_t color = 0;
        std::string label;
        std::string value;
        bool lineKey = false;
    };

    // Draws everything inside (x, y, w, h), absolute.
    virtual void DrawChart(IRenderer* renderer, float x, float y, float w, float h) = 0;
    // The mark under a point given in the view's own coordinates.
    virtual bool HitTest(float localX, float localY, int& series, int& index) const = 0;
    // Legend entries (default: one per series).
    virtual std::vector<LegendEntry> LegendEntries() const;
    virtual ChartLegend AutoLegend(float w, float h, size_t entries) const;
    // A legend entry was clicked (default: show / hide that series).
    virtual void OnLegendClicked(int id);
    // The point the events report for (series, index).
    virtual ChartPointEventArgs PointArgs(int series, int index) const;

    uint32_t PartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    uint32_t NamedPartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    uint32_t PartFill(const char* part, uint32_t fallback) const;
    std::string FontFamily() const;
    float TextSize() const;  // axis and legend text
    uint32_t ResolveColor(const std::string& spec, uint32_t fallback) const;

    // Lays out the title and the legend; returns what is left for the plot.
    UIRect LayoutFrame(IRenderer* renderer, float x, float y, float w, float h);
    void DrawFrame(IRenderer* renderer);  // title and legend as laid out
    // A tooltip near (anchorX, anchorY), kept inside the view.
    void DrawTooltip(IRenderer* renderer, float anchorX, float anchorY, const std::string& title,
                     const std::vector<TooltipRow>& rows) const;
    // Reference lines across the plot along the value axis.
    void DrawThresholds(IRenderer* renderer, const UIRect& plot, const ChartAxis& axis, bool vertical) const;
    // Text right / center / left of a point (no clipping by the mark).
    void DrawLabel(IRenderer* renderer, const std::string& text, float x, float y, float w, float h, uint32_t color,
                   int alignH) const;
    float MeasureWidth(IRenderer* renderer, const std::string& text) const;
    // The text cut to maxW with "..." at the end (whole characters).
    std::string FitText(IRenderer* renderer, const std::string& text, float maxW) const;
    // "No data" in the middle of the plot.
    void DrawEmpty(IRenderer* renderer, const UIRect& area) const;
    int ValueDecimals(double value) const;
    bool HasData() const;
    // Ends a glide: what is drawn is the data now (live appends).
    void SnapToData();

    std::vector<std::string> m_categories;
    std::vector<ChartSeries> m_series;
    std::string m_caption;
    ChartLegend m_legend = ChartLegend::Auto;
    bool m_showValues = false;
    int m_decimals = -1;
    std::string m_unit;
    bool m_localeNumbers = true;
    std::vector<ChartThreshold> m_thresholds;
    bool m_animate = true;

    // The last paint: where the view was (absolute), for hit tests.
    float m_originX = 0.0f;
    float m_originY = 0.0f;
    bool m_hasPointer = false;
    float m_pointerX = 0.0f;  // view coordinates
    float m_pointerY = 0.0f;
    int m_hoverSeries = -1;
    int m_hoverIndex = -1;

private:
    class PaintItem;
    std::shared_ptr<PaintItem> m_paint;
    std::vector<std::vector<double>> m_from;  // values drawn when the data changed
    float m_progress = 1.0f;                  // 0..1 from m_from to the data
    std::vector<LegendEntry> m_legendLayout;
    UIRect m_titleRect;

    void LayoutParts();
    // Call before changing data: the drawn values become the start of a glide.
    void BeginChange();
    void EndChange();
};

} // namespace jwgui
