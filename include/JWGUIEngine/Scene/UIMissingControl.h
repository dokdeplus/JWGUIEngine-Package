#pragma once

#include "../Core/UIRenderView.h"
#include <string>

namespace jwgui {

// Stands in for a scene node whose type is not registered in this program:
// an app's own control opened in the editor or in JWGUIPlayer. A dashed box
// with the type name, so the node keeps its place, can be selected, moved and
// sized, and its attributes stay in UISceneNode::props. The app that
// registers the type (ControlRegistry::Register) gets the real control.
class JWGUI_API UIMissingControl : public UIRenderView {
public:
    explicit UIMissingControl(const std::string& sceneType);

    // The type the scene asked for ("Board2048").
    const std::string& SceneType() const { return m_sceneType; }

    void RenderAll(IRenderer* renderer, float offsetX, float offsetY) override;
    void OnStyleChanged() override;

private:
    std::string m_sceneType;
    // The theme's text color and font, refilled when the style changes.
    uint32_t m_color = 0xFF909090;
    std::string m_family;
};

} // namespace jwgui
