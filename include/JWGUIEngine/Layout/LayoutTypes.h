#pragma once

#include <limits>
#include <vector>
#include "../Core/CoreTypes.h"

namespace jwgui {

struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// ---------------------------------------------------------
// General Layout Types
// ---------------------------------------------------------
enum class LayoutType {
    Absolute,
    Flex,
    Grid
};

enum class SizingMode {
    Fixed,
    Auto,     // fills what the parent gives (root, stretch)
    Percent,
    Star,
    Content   // as large as the content: children, or a leaf's measured text / image
};

struct Sizing {
    SizingMode mode = SizingMode::Fixed;
    float value = 0.0f;
};

struct Constraints {
    Size minSize = {0.0f, 0.0f};
    Size maxSize = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
};

// ---------------------------------------------------------
// Flex Layout Types
// ---------------------------------------------------------
enum class FlexDirection {
    Row,
    Column,
    RowReverse,
    ColumnReverse
};

enum class JustifyContent {
    Start,
    End,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class AlignItems {
    Auto,
    Start,
    End,
    Center,
    Stretch,
    Baseline
};

enum class FlexWrap {
    NoWrap,
    Wrap,
    WrapReverse
};

// ---------------------------------------------------------
// Grid Layout Types
// ---------------------------------------------------------
struct GridTrack {
    SizingMode type = SizingMode::Fixed;
    float value = 0.0f;
};

struct GridCell {
    int row = 0;
    int col = 0;
    int rowSpan = 1;
    int colSpan = 1;
};

// ---------------------------------------------------------
// Absolute Layout Types
// ---------------------------------------------------------
enum AnchorFlags : unsigned int {
    AnchorNone = 0,
    AnchorLeft = 1 << 0,
    AnchorTop = 1 << 1,
    AnchorRight = 1 << 2,
    AnchorBottom = 1 << 3,
    AnchorAll = AnchorLeft | AnchorTop | AnchorRight | AnchorBottom
};

// Edge docking for chrome controls (MenuBar, future ToolBar / StatusBar).
enum class DockSide {
    Top,
    Bottom,
    Left,
    Right
};

// ---------------------------------------------------------
// Utility Functions
// ---------------------------------------------------------
JWGUI_API Rect Inset(Rect rect, Thickness padding);
JWGUI_API Rect ApplyMargin(Rect rect, Thickness margin);

} // namespace jwgui
