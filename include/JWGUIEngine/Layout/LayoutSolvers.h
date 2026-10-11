#pragma once

#include "LayoutTypes.h"
#include <vector>

namespace jwgui {

class LayoutNode;

// Track edges of a Grid node inside contentRect (same space as contentRect).
// Column i spans [colStart[i], colEnd[i]]; rows likewise. Gaps sit between tracks.
struct GridTrackLayout {
    std::vector<float> colStart;
    std::vector<float> colEnd;
    std::vector<float> rowStart;
    std::vector<float> rowEnd;
};

JWGUI_API GridTrackLayout ResolveGridTracks(const LayoutNode& grid, Rect contentRect);

// Entry points for solver algorithms.
// These functions are called by LayoutNode::CalculateChildrenLayout().
// They modify the ResultRect of the child nodes.

void SolveFlexLayout(LayoutNode* parent, Rect contentRect);
void SolveGridLayout(LayoutNode* parent, Rect contentRect);
void SolveAbsoluteLayout(LayoutNode* parent, Rect contentRect);

} // namespace jwgui
