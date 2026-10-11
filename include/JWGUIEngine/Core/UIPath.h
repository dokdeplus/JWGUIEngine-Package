#pragma once
#include <vector>
#include "Export.h"
#include "CoreTypes.h"

namespace jwgui {

enum class PathCommandType {
    MoveTo,
    LineTo,
    CubicTo,
    AddRect,
    AddRoundRect,
    AddCircle,
    ArcTo,
    Close
};

struct PathCommand {
    PathCommandType type;
    float pts[8]; // CubicTo: 6; AddRoundRect: x,y,w,h,tl,tr,br,bl
};

class JWGUI_API UIPath {
public:
    UIPath() = default;
    ~UIPath() = default;

    void MoveTo(float x, float y);
    void LineTo(float x, float y);
    void CubicTo(float x1, float y1, float x2, float y2, float x3, float y3);
    
    void AddRect(float x, float y, float w, float h);
    void AddRoundRect(float x, float y, float w, float h, float radius);
    void AddRoundRect(float x, float y, float w, float h, const CornerRadius& radius);
    void AddCircle(float cx, float cy, float radius);
    // Arc of the circle (cx, cy, radius). Degrees: 0 = +x (3 o'clock),
    // positive = clockwise on screen. Joins the current point with a line
    // unless the path is empty or forceMoveTo is set.
    void ArcTo(float cx, float cy, float radius, float startDeg, float sweepDeg, bool forceMoveTo = false);
    
    void Close();
    void Clear();

    bool Contains(float x, float y) const;
    bool IsEmpty() const;

    /// Returns a copy with all coordinates shifted by (dx, dy).
    UIPath Offset(float dx, float dy) const;

    /// Replaces contents with a rect or round-rect shape.
    void SetRectShape(float x, float y, float w, float h, float cornerRadius = 0.0f);

    const std::vector<PathCommand>& GetCommands() const;

private:
    std::vector<PathCommand> m_commands;
};
} // namespace jwgui
