#pragma once

#include "../Core/Export.h"
#include "../Core/Image.h"
#include <string>

namespace jwgui {

class IRenderer;

// A picture that replaces a drawn part of a control (a clock's face or hand,
// a gauge's needle): a Project resource id (`icons.face`, ResourceManager
// ids), read when it is first drawn and again when the id changes. Empty or
// not found = the control draws the part itself.
class JWGUI_API PartImage {
public:
    void SetId(const std::string& id);
    const std::string& Id() const { return m_id; }
    // Loads it on first use; false when there is no picture.
    bool Ready() const;

    // Fits the picture into the square of half side `radius` around (cx, cy),
    // keeping its proportions (a face).
    void DrawFace(IRenderer* renderer, float cx, float cy, float radius) const;
    // Draws the picture standing up (pointing at 12 o'clock) and turned
    // `degrees` clockwise about (cx, cy): its point (pivotX, pivotY), in
    // fractions of its size, sits on the center, and the part above the pivot
    // is `length` long (a hand, a needle).
    void DrawHand(IRenderer* renderer, float cx, float cy, float degrees, float length, float pivotX,
                  float pivotY) const;

    // "0.5,0.85" -> 0.5, 0.85 (each 0..1). False and unchanged when it does not read.
    static bool ParsePivot(const std::string& text, float& x, float& y);

private:
    std::string m_id;
    mutable Image m_image;
    mutable bool m_loaded = false;
};

} // namespace jwgui
