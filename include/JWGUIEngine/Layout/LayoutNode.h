#pragma once

#include "LayoutTypes.h"
#include <functional>
#include <vector>
#include <memory>
#include <optional>

namespace jwgui {

class JWGUI_API LayoutNode : public std::enable_shared_from_this<LayoutNode> {
public:
    LayoutNode();
    virtual ~LayoutNode() = default;

    // Hierarchy
    void AddChild(std::shared_ptr<LayoutNode> child);
    // At `index` (past the end = last).
    void InsertChild(size_t index, std::shared_ptr<LayoutNode> child);
    void RemoveChild(std::shared_ptr<LayoutNode> child);
    void RemoveFromParent();
    const std::vector<std::shared_ptr<LayoutNode>>& GetChildren() const { return m_children; }
    LayoutNode* GetParent() const { return m_parent; }

    // Core Properties
    void SetLayoutType(LayoutType type) { m_layoutType = type; }
    LayoutType GetLayoutType() const { return m_layoutType; }

    void SetMargin(const Thickness& margin) { m_margin = margin; }
    const Thickness& GetMargin() const { return m_margin; }

    void SetPadding(const Thickness& padding) { m_padding = padding; }
    const Thickness& GetPadding() const { return m_padding; }

    void SetWidth(Sizing sizing) { m_width = sizing; }
    const Sizing& GetWidth() const { return m_width; }

    void SetHeight(Sizing sizing) { m_height = sizing; }
    const Sizing& GetHeight() const { return m_height; }

    void SetConstraints(const Constraints& constraints) { m_constraints = constraints; }
    const Constraints& GetConstraints() const { return m_constraints; }
    void SetMinSize(float w, float h) {
        m_constraints.minSize.width = w;
        m_constraints.minSize.height = h;
    }
    void SetMaxSize(float w, float h) {
        m_constraints.maxSize.width = w;
        m_constraints.maxSize.height = h;
    }
    void SetMinWidth(float w) { m_constraints.minSize.width = w; }
    void SetMinHeight(float h) { m_constraints.minSize.height = h; }
    void SetMaxWidth(float w) { m_constraints.maxSize.width = w; }
    void SetMaxHeight(float h) { m_constraints.maxSize.height = h; }

    // Flex Properties
    void SetFlexDirection(FlexDirection dir) { m_flexDirection = dir; }
    FlexDirection GetFlexDirection() const { return m_flexDirection; }

    void SetJustifyContent(JustifyContent jc) { m_justifyContent = jc; }
    JustifyContent GetJustifyContent() const { return m_justifyContent; }

    void SetAlignItems(AlignItems ai) { m_alignItems = ai; }
    AlignItems GetAlignItems() const { return m_alignItems; }

    void SetFlexWrap(FlexWrap wrap) { m_flexWrap = wrap; }
    FlexWrap GetFlexWrap() const { return m_flexWrap; }

    void SetFlexGrow(float grow) { m_flexGrow = grow; }
    float GetFlexGrow() const { return m_flexGrow; }

    void SetFlexShrink(float shrink) { m_flexShrink = shrink; }
    float GetFlexShrink() const { return m_flexShrink; }

    void SetFlexBasis(Sizing basis) { m_flexBasis = basis; }
    const std::optional<Sizing>& GetFlexBasis() const { return m_flexBasis; }

    // Space between children: Flex items on the main axis, Grid rows and columns.
    void SetGap(float gap) { m_gap = gap < 0.0f ? 0.0f : gap; }
    float GetGap() const { return m_gap; }

    // Grid Properties
    void SetGridRow(int row) { m_gridCell.row = row; }
    void SetGridColumn(int col) { m_gridCell.col = col; }
    void SetGridRowSpan(int span) { m_gridCell.rowSpan = span; }
    void SetGridColumnSpan(int span) { m_gridCell.colSpan = span; }
    const GridCell& GetGridCell() const { return m_gridCell; }

    void AddGridRowTrack(const GridTrack& track) { m_gridRows.push_back(track); }
    void AddGridColumnTrack(const GridTrack& track) { m_gridCols.push_back(track); }
    const std::vector<GridTrack>& GetGridRowTracks() const { return m_gridRows; }
    const std::vector<GridTrack>& GetGridColumnTracks() const { return m_gridCols; }
    void ClearGridTracks() { m_gridRows.clear(); m_gridCols.clear(); }

    // Absolute / Anchor Properties
    void SetAnchorFlags(unsigned int flags) { m_anchorFlags = flags; }
    unsigned int GetAnchorFlags() const { return m_anchorFlags; }

    void SetAbsolutePosition(Point pos) { m_absolutePos = pos; }
    const Point& GetAbsolutePosition() const { return m_absolutePos; }

    // Calculation & Results
    void CalculateLayout(Size availableSize);
    void CalculateLayout(Rect availableRect);

    // This is internally set by solvers during CalculateLayout
    void SetResultRect(const Rect& rect);
    const Rect& GetResultRect() const { return m_resultRect; }

    // Utility for solvers
    Size ResolveDesiredSize(Size availableSize) const;

    // SizingMode::Content. A leaf asks its measure function (a view's text or
    // image, or a child panel's own layout); a container adds up its children
    // as its layout type places them (Flex: along the main axis plus gaps,
    // Grid: widest / tallest per track, Absolute: the farthest edge), plus
    // padding. Children sized Star / Auto / Percent count as 0.
    using MeasureFunc = std::function<Size()>;
    void SetMeasureFunc(MeasureFunc fn) { m_measure = std::move(fn); }
    Size MeasureContent() const;

private:
    void CalculateChildrenLayout(Rect contentRect);

    LayoutNode* m_parent = nullptr;
    std::vector<std::shared_ptr<LayoutNode>> m_children;

    LayoutType m_layoutType = LayoutType::Flex;

    Thickness m_margin;
    Thickness m_padding;
    Sizing m_width = {SizingMode::Auto, 0.0f};
    Sizing m_height = {SizingMode::Auto, 0.0f};
    Constraints m_constraints;

    // Flex properties
    FlexDirection m_flexDirection = FlexDirection::Row;
    JustifyContent m_justifyContent = JustifyContent::Start;
    AlignItems m_alignItems = AlignItems::Stretch;
    FlexWrap m_flexWrap = FlexWrap::NoWrap;
    float m_flexGrow = 0.0f;
    float m_flexShrink = 1.0f;
    std::optional<Sizing> m_flexBasis;
    float m_gap = 0.0f;

    // Grid properties
    GridCell m_gridCell;
    std::vector<GridTrack> m_gridRows;
    std::vector<GridTrack> m_gridCols;

    // Absolute properties
    unsigned int m_anchorFlags = AnchorNone;
    Point m_absolutePos;

    MeasureFunc m_measure;

    // Result
    Rect m_resultRect;
};

} // namespace jwgui
