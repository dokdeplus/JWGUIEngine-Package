#pragma once

#include "UISceneNode.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIScene {
public:
    std::string id = "Main";
    float width = 800.0f;
    float height = 600.0f;
    std::shared_ptr<UISceneNode> root;

    static std::shared_ptr<UIScene> CreateDefault();

    bool Save(const char* path, std::string* error = nullptr) const;
    bool Load(const char* path, std::string* error = nullptr);

    std::string SaveToString() const;
    bool LoadFromString(const std::string& xml);
};

// Nodes as XML (copy / paste): <JWGUINodes version="1"> with one element per
// node, children included, in the same format as a scene.
JWGUI_API std::string SaveNodesToString(const std::vector<const UISceneNode*>& nodes);
// False when the text is not a JWGUINodes document.
JWGUI_API bool LoadNodesFromString(const std::string& xml, std::vector<std::shared_ptr<UISceneNode>>& out);

} // namespace jwgui
