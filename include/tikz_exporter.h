#pragma once
#include <string>
#include <entt/entt.hpp>
#include "graphcanvas.h"  // for Position/Label structs

class TikZExporter
{
public:
    static std::string exportToTikZ(const entt::registry& reg);
};