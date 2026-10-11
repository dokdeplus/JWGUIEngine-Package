#pragma once

#include "../Core/Export.h"
#include "../Core/UIRenderView.h"
#include <memory>
#include <string>

namespace jwgui {

class JWGUI_API ControlFactory {
public:
    static std::shared_ptr<UIRenderView> Create(const std::string& type);
    static void DefaultSize(const std::string& type, float& width, float& height);
    static std::string DefaultText(const std::string& type);
};

} // namespace jwgui
