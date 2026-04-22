#include "graph_renderer.h"
#include <QLineF>

void GraphRenderer::drawWorkspace(QPainter* painter, const QRectF& /*bounds*/) {
    // Drop Shadow for Document
    painter->fillRect(QRectF(2, 2, 210, 297), QColor(0, 0, 0, 50));
    // White A4 Document
    painter->fillRect(QRectF(0, 0, 210, 297), Qt::white);

    // Page border (A4)
    painter->setPen(QPen(Qt::black, 1));
    painter->drawRect(0, 0, 210, 297);

    // Page margins (e.g., 30mm)
    painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter->drawRect(30, 30, 210-60, 297-60);

    // Grid (10mm)
    painter->setPen(QPen(QColor(220, 220, 220, 100), 1));
    for (int x = 10; x < 210; x += 10) painter->drawLine(x, 0, x, 297);
    for (int y = 10; y < 297; y += 10) painter->drawLine(0, y, 210, y);
}

void GraphRenderer::drawGraph(QPainter* painter, const entt::registry& reg) {
    // Draw edges
    reg.view<Edge>().each([&](auto /*entity*/, const auto& edge) {
        if (reg.valid(edge.source) && reg.valid(edge.target)) {
            const auto& p1 = reg.get<Position>(edge.source);
            const auto& p2 = reg.get<Position>(edge.target);
            painter->setPen(QPen(Qt::darkGray, 2));
            painter->drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
        }
    });

    // Draw nodes
    reg.view<Position, Label, Style>().each([&](auto entity, const auto& pos, const auto& label, const auto& style) {
        if (!style.borderless) {
            painter->setBrush(QColor(style.shade, style.shade, style.shade));
            if (reg.all_of<Selected>(entity)) {
                painter->setPen(QPen(QColor("#3399FF"), 3));
            } else {
                painter->setPen(QPen(Qt::black, 2));
            }
            painter->drawEllipse(QPointF(pos.x, pos.y), style.radius, style.radius);
        } else {
            if (reg.all_of<Selected>(entity)) {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor(51, 153, 255, 150), 2, Qt::DashLine));
                painter->drawEllipse(QPointF(pos.x, pos.y), style.radius, style.radius);
            }
        }

        QRectF textRect;
        int align = Qt::AlignCenter;
        float pd = 4.0f;

        switch(label.pos) {
            case LabelPosition::Center:
                textRect = QRectF(pos.x-style.radius, pos.y-style.radius, style.radius*2, style.radius*2);
                painter->setPen(style.shade < 128 ? Qt::white : Qt::black);
                break;
            case LabelPosition::Top:
                textRect = QRectF(pos.x-style.radius*3, pos.y-style.radius-20-pd, style.radius*6, 20);
                align = Qt::AlignBottom | Qt::AlignHCenter;
                painter->setPen(Qt::black);
                break;
            case LabelPosition::Bottom:
                textRect = QRectF(pos.x-style.radius*3, pos.y+style.radius+pd, style.radius*6, 20);
                align = Qt::AlignTop | Qt::AlignHCenter;
                painter->setPen(Qt::black);
                break;
            case LabelPosition::Left:
                textRect = QRectF(pos.x-style.radius-40-pd, pos.y-style.radius, 40, style.radius*2);
                align = Qt::AlignRight | Qt::AlignVCenter;
                painter->setPen(Qt::black);
                break;
            case LabelPosition::Right:
                textRect = QRectF(pos.x+style.radius+pd, pos.y-style.radius, 40, style.radius*2);
                align = Qt::AlignLeft | Qt::AlignVCenter;
                painter->setPen(Qt::black);
                break;
        }

        painter->drawText(textRect, align, QString::fromStdString(label.text));
    });
}

void GraphRenderer::drawSelectionBox(QPainter* painter, const QPointF& start, const QPointF& end) {
    painter->setPen(QPen(QColor("#3399FF"), 2, Qt::DashLine));
    painter->setBrush(QColor(51, 153, 255, 30));
    QRectF sel = QRectF(start, end).normalized();
    painter->drawRect(sel);
}

void GraphRenderer::drawPendingEdge(QPainter* painter, const QPointF& start, const QPointF& end) {
    painter->setPen(QPen(Qt::green, 2, Qt::DashLine));
    painter->drawLine(start, end);
}
