#pragma once

#include "UIScene.h"
#include "../Core/UIStyle.h"
#include "../Controls/UIPanel.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UILoader {
public:
    using NavigateFn = std::function<void(const std::string& sceneId)>;

    static std::shared_ptr<UIRenderView> Instantiate(const UIScene& scene, std::shared_ptr<UIStyle> style);
    static std::shared_ptr<UIRenderView> Instantiate(const UIScene& scene, std::shared_ptr<UIStyle> style,
                                                     float width, float height);
    // parentNode gives the design size that anchored Absolute children keep
    // their edge distances against.
    static void ApplyGeometry(UIPanel* parent, UIRenderView* child, const UISceneNode& node,
                              const UISceneNode* parentNode = nullptr);
    // "200, 1*, 2*, 30%" -> count tracks; missing or unreadable entries are 1*.
    static std::vector<GridTrack> ParseGridTracks(const std::string& sizes, int count);
    static void ApplyContainerLayout(UIPanel* panel, const UISceneNode& node);
    // `text` as written in the scene: "@id" texts show the string they name.
    static void ApplyText(UIRenderView* view, const std::string& text);
    static void ApplyInstance(UIRenderView* view, const UISceneNode& node);
    // Gives the "@id" texts of every live view again (UIStrings' language
    // changed); runs by itself on a change, and lays the window out again.
    static void RefreshTexts();
    static void BindNavigation(UIRenderView* liveRoot, const UISceneNode& node, NavigateFn fn);

private:
    static std::shared_ptr<UIRenderView> InstantiateNode(const UISceneNode& node, std::shared_ptr<UIStyle> style);
    static void AttachChildren(UIPanel* parent, const UISceneNode& node, std::shared_ptr<UIStyle> style);
};

} // namespace jwgui
