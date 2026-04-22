#pragma once
#include <QPainter>
#include <entt/entt.hpp>
#include "components.h"

class GraphRenderer {
public:
    static void drawWorkspace(QPainter* painter, const QRectF& bounds);
    static void drawGraph(QPainter* painter, const entt::registry& reg);
    static void drawSelectionBox(QPainter* painter, const QPointF& start, const QPointF& end);
    static void drawPendingEdge(QPainter* painter, const QPointF& start, const QPointF& end);
};
