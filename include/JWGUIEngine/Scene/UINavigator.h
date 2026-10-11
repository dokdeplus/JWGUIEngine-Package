#pragma once

#include "UIScenePack.h"
#include "../Core/UIStyle.h"
#include "../Core/UIRenderView.h"
#include <functional>
#include <memory>
#include <string>

namespace jwgui {

class JWGUI_API UINavigator {
public:
    void Bind(std::shared_ptr<UIScenePack> pack, std::shared_ptr<UIStyle> style);
    bool NavigateTo(const std::string& sceneId);
    bool NavigateToStart();

    std::shared_ptr<UIRenderView> LiveRoot() const { return m_liveRoot; }
    const UIScene* CurrentScene() const;
    const std::string& CurrentId() const { return m_currentId; }

    std::function<void()> Changed;

private:
    std::shared_ptr<UIScenePack> m_pack;
    std::shared_ptr<UIStyle> m_style;
    std::shared_ptr<UIRenderView> m_liveRoot;
    std::string m_currentId;
};

} // namespace jwgui
