#pragma once

#include "Export.h"
#include "IPlatformHost.h"
#include "IRenderer.h"
#include <memory>

namespace jwgui {

JWGUI_API std::shared_ptr<IPlatformHost> CreatePlatformHost();
JWGUI_API std::shared_ptr<IRenderer> CreateRenderer();

} // namespace jwgui
