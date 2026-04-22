#pragma once
#include <QString>
#include <entt/entt.hpp>

class GraphSerializer {
public:
    static QString toJSON(const entt::registry& reg);
    static QString toCSV(const entt::registry& reg);
    
    static void fromJSON(entt::registry& reg, const QString& jsonString);
    static void fromCSV(entt::registry& reg, const QString& csvString);
    
    static QString extractJSONFromSVG(const QString& svgString);
};
