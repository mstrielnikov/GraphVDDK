#include "graphcanvas.h"
#include <QPainter>
#include <QLineF>
#include <QFile>
#include <QTextStream>
#include <QGuiApplication>
#include <QClipboard>
#include <QUrl>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QHoverEvent>
#include <QTimer>
#include <vector>
#include "graphexamples.h"
#include "tikz_exporter.h"
#include <QSvgGenerator>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>

// Constants
constexpr float GRID_SIZE = 10.0f;

GraphCanvas::GraphCanvas(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);  // ← OpenGL option ON
    setAntialiasing(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptHoverEvents(true);

    m_zoom = 3.5f;
    m_pan = QPointF(50.f, 50.f);

    m_edgePanTimer = new QTimer(this);
    connect(m_edgePanTimer, &QTimer::timeout, this, [this]() {
        if (m_zoomScaleFactor <= 1.0f) return;
        float margin = 50.0f;
        float speed = 15.0f;
        float cw = boundingRect().width();
        float ch = boundingRect().height();
        
        bool panned = false;
        if (m_lastHoverPos.x() < margin) { m_panExtraOffset.setX(m_panExtraOffset.x() + speed); panned = true; }
        else if (m_lastHoverPos.x() > cw - margin) { m_panExtraOffset.setX(m_panExtraOffset.x() - speed); panned = true; }
        
        if (m_lastHoverPos.y() < margin) { m_panExtraOffset.setY(m_panExtraOffset.y() + speed); panned = true; }
        else if (m_lastHoverPos.y() > ch - margin) { m_panExtraOffset.setY(m_panExtraOffset.y() - speed); panned = true; }
        
        if (panned) update();
    });
    m_edgePanTimer->start(16);

    saveSnapshot(); // Base history node
}

void GraphCanvas::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // Workspace Background
    painter->fillRect(boundingRect(), QColor("#E0E0E0"));

    // Auto-Align Workspace: Center the 210x297 A4 Document gracefully inside the physical canvas boundary
    float cw = boundingRect().width();
    float ch = boundingRect().height();
    if (cw > 0 && ch > 0) {
        float targetW = cw - 40.0f; // 20px physical padding
        float targetH = ch - 40.0f;
        float oldZoom = m_zoom;
        
        float baseZoom = std::min(targetW / 210.0f, targetH / 297.0f);
        m_zoom = baseZoom * m_zoomScaleFactor;
        
        if (m_zoom <= 0.1f) m_zoom = 0.1f;
        if (m_zoom != oldZoom) {
            emit zoomChanged();
        }
        
        // Compute rigid limits mapping A4 overlays natively
        float docScreenW = 210.0f * m_zoom;
        float docScreenH = 297.0f * m_zoom;
        
        // Clamp offsets bounding limits strictly to visible window
        float minX = std::min(0.0f, cw - 20.0f - docScreenW);
        float maxX = std::max(0.0f, 20.0f - (cw - docScreenW)/2.0f);
        float minY = std::min(0.0f, ch - 20.0f - docScreenH);
        float maxY = std::max(0.0f, 20.0f - (ch - docScreenH)/2.0f);
        
        if (m_zoomScaleFactor <= 1.0f) {
            m_panExtraOffset = QPointF(0, 0); // Disable nav when fully fit
        } else {
            m_panExtraOffset.setX(std::clamp<float>(static_cast<float>(m_panExtraOffset.x()), minX, maxX));
            m_panExtraOffset.setY(std::clamp<float>(static_cast<float>(m_panExtraOffset.y()), minY, maxY));
        }

        m_pan.setX(((cw - docScreenW) / 2.0f) + m_panExtraOffset.x());
        m_pan.setY(((ch - docScreenH) / 2.0f) + m_panExtraOffset.y());
    }

    painter->save();
    painter->translate(m_pan);
    painter->scale(m_zoom, m_zoom);

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

    drawGraph(painter);

    if (m_drawingEdgeSource != entt::null) {
        const auto& p1 = m_registry.get<Position>(m_drawingEdgeSource);
        painter->setPen(QPen(Qt::green, 2, Qt::DashLine));
        painter->drawLine(QPointF(p1.x, p1.y), m_drawingEdgeEnd);
    }
    
    painter->restore();
}

QPointF GraphCanvas::mapToScene(const QPointF& pos) const
{
    return (pos - m_pan) / m_zoom;
}

void GraphCanvas::drawGraph(QPainter* painter) const
{
    // Draw edges
    m_registry.view<Edge>().each([&](auto /*entity*/, const auto& edge) {
        if (m_registry.valid(edge.source) && m_registry.valid(edge.target)) {
            const auto& p1 = m_registry.get<Position>(edge.source);
            const auto& p2 = m_registry.get<Position>(edge.target);
            painter->setPen(QPen(Qt::darkGray, 2));
            painter->drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
        }
    });

    if (m_drawingEdgeSource != entt::null && m_registry.valid(m_drawingEdgeSource)) {
        const auto& p1 = m_registry.get<Position>(m_drawingEdgeSource);
        painter->setPen(QPen(Qt::green, 2, Qt::DashLine));
        painter->drawLine(QPointF(p1.x, p1.y), m_drawingEdgeEnd);
    }

    // Draw nodes
    m_registry.view<Position, Label, Style>().each([&](auto entity, const auto& pos, const auto& label, const auto& style) {
        if (!style.borderless) {
            painter->setBrush(QColor(style.shade, style.shade, style.shade));
            if (m_registry.all_of<Selected>(entity)) {
                painter->setPen(QPen(QColor("#3399FF"), 3));
            } else {
                painter->setPen(QPen(Qt::black, 2));
            }
            painter->drawEllipse(QPointF(pos.x, pos.y), style.radius, style.radius);
        } else {
            if (m_registry.all_of<Selected>(entity)) {
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

    if (m_isBoxSelecting) {
        painter->setPen(QPen(QColor("#3399FF"), 2, Qt::DashLine));
        painter->setBrush(QColor(51, 153, 255, 30));
        QRectF sel = QRectF(m_boxSelectStart, m_boxSelectEnd).normalized();
        painter->drawRect(sel);
    }
}


void GraphCanvas::mousePressEvent(QMouseEvent* event)
{
    if (m_drawingEdgeSource != entt::null) {
        if (event->button() == Qt::LeftButton) {
            QPointF pos = mapToScene(event->position());
            entt::entity targetNode = entt::null;
            m_registry.view<Position, Style>().each([&](auto entity, const auto& p, const auto& s) {
                if (QLineF(QPointF(p.x, p.y), pos).length() < s.radius) {
                    targetNode = entity;
                }
            });

            if (targetNode != entt::null && targetNode != m_drawingEdgeSource) {
                auto edgeEntity = m_registry.create();
                m_registry.emplace<Edge>(edgeEntity, m_drawingEdgeSource, targetNode);
                saveSnapshot();
            }
            m_drawingEdgeSource = entt::null;
            update();
            return;
        }
    }

    if (event->button() == Qt::RightButton) {
        QPointF pos = mapToScene(event->position());
        entt::entity clickedNode = entt::null;
        m_registry.view<Position, Style>().each([&](auto entity, const auto& p, const auto& s) {
            if (QLineF(QPointF(p.x, p.y), pos).length() < s.radius) {
                clickedNode = entity;
            }
        });
        if (clickedNode != entt::null) {
            QString currentText = QString::fromStdString(m_registry.get<Label>(clickedNode).text);
            emit nodeRightClicked(static_cast<int>(clickedNode), currentText, event->position().x(), event->position().y());
            return;
        }
        
        // Context menu on Right-Click empty background
        emit canvasRightClicked(event->position().x(), event->position().y());
        return;
    }

    QPointF pos = mapToScene(event->position());
    entt::entity clickedNode = entt::null;
    // Simple hit-test (expand for full ports later)
    m_registry.view<Position, Style>().each([&](auto entity, const auto& p, const auto& s) {
        if (QLineF(QPointF(p.x, p.y), pos).length() < s.radius) {
            clickedNode = entity;
        }
    });

    if (clickedNode != entt::null) {
        if (event->modifiers() & Qt::ShiftModifier) {
            m_drawingEdgeSource = clickedNode;
            m_drawingEdgeEnd = pos;
        } else {
            if (event->modifiers() & Qt::ControlModifier) {
                if (m_registry.all_of<Selected>(clickedNode)) {
                    m_registry.remove<Selected>(clickedNode);
                } else {
                    m_registry.emplace<Selected>(clickedNode);
                }
                m_dragged = entt::null;
            } else {
                if (!m_registry.all_of<Selected>(clickedNode)) {
                    m_registry.clear<Selected>();
                    m_registry.emplace<Selected>(clickedNode);
                }
                m_dragged = clickedNode;
                m_dragSceneStart = pos;
                m_dragStartPositions.clear();
                m_registry.view<Selected, Position>().each([&](auto e, const auto& p) {
                    m_dragStartPositions[e] = QPointF(p.x, p.y);
                });
            }
            update();
        }
    } else {
        if (!(event->modifiers() & Qt::ControlModifier)) {
            m_registry.clear<Selected>();
        }
        m_isBoxSelecting = true;
        m_boxSelectStart = pos;
        m_boxSelectEnd = pos;
        update();
    }
}

void GraphCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isBoxSelecting) {
        m_boxSelectEnd = mapToScene(event->position());
        update();
        return;
    }

    if (m_drawingEdgeSource != entt::null) {
        m_drawingEdgeEnd = mapToScene(event->position());
        update();
        return;
    }

    if (m_dragged != entt::null) {
        QPointF pos = mapToScene(event->position());
        QPointF totalDelta = pos - m_dragSceneStart;
        
        for (const auto& [e, startPos] : m_dragStartPositions) {
            if (m_registry.valid(e)) {
                auto& p = m_registry.get<Position>(e);
                p.x = std::round((startPos.x() + totalDelta.x()) / 10.0f) * 10.0f;
                p.y = std::round((startPos.y() + totalDelta.y()) / 10.0f) * 10.0f;
            }
        }
        update();
    }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isBoxSelecting) {
        m_isBoxSelecting = false;
        QRectF selectionRect = QRectF(m_boxSelectStart, m_boxSelectEnd).normalized();
        m_registry.view<Position, Style>().each([&](auto entity, const auto& p, const auto& s) {
            if (selectionRect.intersects(QRectF(p.x - s.radius, p.y - s.radius, s.radius * 2, s.radius * 2))) {
                if (!m_registry.all_of<Selected>(entity)) {
                    m_registry.emplace<Selected>(entity);
                }
            }
        });
        update();
        return;
    }

    if (m_drawingEdgeSource != entt::null) {
        QPointF pos = mapToScene(event->position());
        entt::entity targetNode = entt::null;
        m_registry.view<Position, Style>().each([&](auto entity, const auto& p, const auto& s) {
            if (QLineF(QPointF(p.x, p.y), pos).length() < s.radius) {
                targetNode = entity;
            }
        });

        if (targetNode == m_drawingEdgeSource) {
            // Releasing exactly where we started drawing does NOT count as a cancellation - let it hover!
            return;
        }

        if (targetNode != entt::null) {
            auto edgeEntity = m_registry.create();
            m_registry.emplace<Edge>(edgeEntity, m_drawingEdgeSource, targetNode);
            saveSnapshot();
        }
        
        m_drawingEdgeSource = entt::null;
        update();
    }

    if (m_dragged != entt::null) {
        saveSnapshot();
        m_dragged = entt::null;
    }
}

void GraphCanvas::wheelEvent(QWheelEvent* event)
{
    // Zooming disabled natively in favor of standard A4 workspace auto-alignment scaling loops
    event->accept();
}

void GraphCanvas::saveStringToFile(const QString& content, const QUrl& url) const {
    if (url.isEmpty()) return;
    QFile file(url.toLocalFile());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
    }
}

void GraphCanvas::copyToClipboard(const QString& content) const {
    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(content);
    }
}

QString GraphCanvas::exportToTikZ() const {
    return QString::fromStdString(TikZExporter::exportToTikZ(m_registry));
}

QString GraphCanvas::exportToSVG() const {
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);
    generator.setSize(QSize(210, 297));
    generator.setViewBox(QRect(0, 0, 210, 297));
    QPainter painter(&generator);
    drawGraph(&painter);
    painter.end();
    QString svgData = QString::fromUtf8(buffer.data());
    
    // Inject JSON state mapping losslessly natively
    QString jsonState = exportToJSON();
    svgData.replace("</svg>", QString("  <!-- GRAPH_DATA_BEGIN\n%1\n  GRAPH_DATA_END -->\n</svg>").arg(jsonState));
    
    return svgData;
}

QString GraphCanvas::exportToJSON() const {
    QJsonArray nodesArr;
    m_registry.view<Position, Label, Style>().each([&](auto e, const auto& p, const auto& l, const auto& s) {
        QJsonObject n;
        n["id"] = (int)e;
        n["x"] = p.x; n["y"] = p.y;
        n["label"] = QString::fromStdString(l.text);
        n["labelPos"] = static_cast<int>(l.pos);
        n["shade"] = s.shade;
        n["radius"] = s.radius;
        n["borderless"] = s.borderless;
        nodesArr.append(n);
    });
    QJsonArray edgesArr;
    m_registry.view<Edge>().each([&](auto /*e*/, const auto& edge) {
        QJsonObject o; o["source"] = (int)edge.source; o["target"] = (int)edge.target;
        edgesArr.append(o);
    });
    QJsonObject docObj;
    docObj["nodes"] = nodesArr;
    docObj["edges"] = edgesArr;
    QJsonDocument doc(docObj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void GraphCanvas::exportToPNG(const QString& filePath) const {
    QImage image(210 * 4, 297 * 4, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.scale(4, 4); // scale up for high-res PNG
    drawGraph(&painter);
    painter.end();
    image.save(filePath);
}

void GraphCanvas::addNode(float x, float y, int shade, float radius)
{
    QPointF pos = mapToScene(QPointF(x, y));
    float tx = std::round(pos.x() / 10.0f) * 10.0f;
    float ty = std::round(pos.y() / 10.0f) * 10.0f;

    m_registry.clear<Selected>();
    auto n = m_registry.create();
    m_registry.emplace<Position>(n, tx, ty);
    m_registry.emplace<Label>(n, "");
    m_registry.emplace<Style>(n, shade, radius);
    m_registry.emplace<Selected>(n);

    saveSnapshot();
    update();
}

void GraphCanvas::setNodeText(int entityId, const QString& text) {
    if (m_registry.valid((entt::entity)entityId)) {
        auto& l = m_registry.get<Label>((entt::entity)entityId);
        l.text = text.toStdString();
        update();
    }
}

void GraphCanvas::toggleNodeBorderless(int entityId) {
    if (m_registry.valid((entt::entity)entityId)) {
        auto& s = m_registry.get<Style>((entt::entity)entityId);
        s.borderless = !s.borderless;
        saveSnapshot();
        update();
    }
}

void GraphCanvas::setNodeLabelPosition(int entityId, int posIndex) {
    if (m_registry.valid((entt::entity)entityId)) {
        auto& l = m_registry.get<Label>((entt::entity)entityId);
        l.pos = static_cast<LabelPosition>(posIndex);
        update();
    }
}

int GraphCanvas::getNodeLabelPosition(int entityId) const {
    if (m_registry.valid((entt::entity)entityId)) {
        return static_cast<int>(m_registry.get<Label>((entt::entity)entityId).pos);
    }
    return 0;
}



void GraphCanvas::zoomIn() {
    m_zoomScaleFactor *= 1.25f;
    update();
}

void GraphCanvas::zoomOut() {
    m_zoomScaleFactor /= 1.25f;
    if (m_zoomScaleFactor < 1.0f) m_zoomScaleFactor = 1.0f;
    update();
}

void GraphCanvas::setZoomScale(float percent) {
    float cw = boundingRect().width();
    float ch = boundingRect().height();
    if (cw <= 0 || ch <= 0) return;
    
    float targetW = cw - 40.0f;
    float targetH = ch - 40.0f;
    float baseZoom = std::min(targetW / 210.0f, targetH / 297.0f);
    
    m_zoomScaleFactor = (percent / 100.0f) / baseZoom;
    if (m_zoomScaleFactor < 1.0f) m_zoomScaleFactor = 1.0f; // clamp so it won't shrink smaller than AutoFit
    update();
}

void GraphCanvas::autoAlign(int type) {
    if (m_registry.view<Position>().empty()) return;
    
    std::vector<entt::entity> nodes;
    m_registry.view<Position>().each([&](auto entity, auto&) { nodes.push_back(entity); });
    
    if (type == 0) { // Radial
        float cx = 210.0f / 2.0f;
        float cy = 297.0f / 2.0f; 
        float radius = std::min(90.0f, 30.0f + nodes.size() * 4.0f);
        float angleStep = 2.0f * 3.14159f / nodes.size();
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& p = m_registry.get<Position>(nodes[i]);
            p.x = static_cast<float>(cx + radius * std::cos(i * angleStep));
            p.y = static_cast<float>(cy + radius * std::sin(i * angleStep));
        }
    } else if (type == 1) { // Row-wise
        float startX = 30.0f;
        float startY = 30.0f;
        float spacingX = std::min(50.0f, 180.0f / std::max(1.0f, static_cast<float>(std::ceil(std::sqrt(nodes.size())))));
        float spacingY = spacingX;
        int cols = std::max(1, static_cast<int>(std::ceil(std::sqrt(nodes.size()))));
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& p = m_registry.get<Position>(nodes[i]);
            p.x = startX + (i % cols) * spacingX;
            p.y = startY + (i / cols) * spacingY;
        }
    } else if (type == 2) { // Columnar
        float startX = 30.0f;
        float startY = 30.0f;
        float spacingX = std::min(50.0f, 180.0f / std::max(1.0f, static_cast<float>(std::ceil(std::sqrt(nodes.size())))));
        float spacingY = spacingX;
        int rows = std::max(1, static_cast<int>(std::ceil(std::sqrt(nodes.size()))));
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& p = m_registry.get<Position>(nodes[i]);
            p.x = startX + (i / rows) * spacingX;
            p.y = startY + (i % rows) * spacingY;
        }
    }
    

    saveSnapshot();
    update();
}



void GraphCanvas::copy() {
    m_clipboard.clear();
    m_registry.view<Selected, Position, Style, Label>().each([&](auto, const auto& p, const auto& s, const auto& l) {
        m_clipboard.push_back({p.x, p.y, s.shade, s.radius, l.text, l.pos, s.borderless});
    });
}

void GraphCanvas::paste() {
    if (m_clipboard.empty()) return;

    m_registry.clear<Selected>();
    float offsetX = 20.0f;
    float offsetY = 20.0f;

    for (const auto& n : m_clipboard) {
        auto e = m_registry.create();
        m_registry.emplace<Position>(e, n.x + offsetX, n.y + offsetY);
        m_registry.emplace<Style>(e, n.shade, n.radius, n.borderless);
        m_registry.emplace<Label>(e, n.text, n.pos);
        m_registry.emplace<Selected>(e);
    }
    
    for (auto& n : m_clipboard) { n.x += offsetX; n.y += offsetY; }
    
    saveSnapshot();
    update();
}

void GraphCanvas::selectAll() {
    m_registry.view<Position>().each([&](auto e, const auto&) {
        if (!m_registry.all_of<Selected>(e)) m_registry.emplace<Selected>(e);
    });
    update();
}

void GraphCanvas::saveSnapshot() {
    Snapshot snap;
    m_registry.view<Position, Label, Style>().each([&](auto e, const auto& p, const auto& l, const auto& s) {
        snap.nodes.push_back({(int)e, p.x, p.y, s.shade, s.radius, l.text, l.pos, s.borderless});
    });
    m_registry.view<Edge>().each([&](auto, const auto& edge) {
        snap.edges.push_back({(int)edge.source, (int)edge.target});
    });

    if (m_historyIndex >= 0 && m_historyIndex < (int)m_history.size() - 1) {
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }

    m_history.push_back(snap);
    m_historyIndex = m_history.size() - 1;
    
    emit graphChanged();
}

void GraphCanvas::undo() {
    if (m_historyIndex > 0) {
        m_historyIndex--;
        const auto& snap = m_history[m_historyIndex];

        m_registry.clear();
        m_dragged = entt::null;
        m_drawingEdgeSource = entt::null;
        m_dragStartPositions.clear();

        std::unordered_map<int, entt::entity> mapping;
        for (const auto& n : snap.nodes) {
            auto entity = m_registry.create();
            m_registry.emplace<Position>(entity, n.x, n.y);
            m_registry.emplace<Style>(entity, n.shade, n.radius);
            m_registry.emplace<Label>(entity, n.text, n.pos);
            if (m_clipboard.empty()) {} // placeholder use
            mapping[n.id] = entity;
        }

        for (const auto& e : snap.edges) {
            auto entity = m_registry.create();
            if (mapping.count(e.sourceId) && mapping.count(e.targetId)) {
                m_registry.emplace<Edge>(entity, mapping[e.sourceId], mapping[e.targetId]);
            }
        }
    
        update();
    }
}

void GraphCanvas::redo() {
    if (m_historyIndex < static_cast<int>(m_history.size()) - 1) {
        m_historyIndex++;
        const auto& snap = m_history[m_historyIndex];

        m_registry.clear();
        m_dragged = entt::null;
        m_drawingEdgeSource = entt::null;
        m_dragStartPositions.clear();

        std::unordered_map<int, entt::entity> mapping;
        for (const auto& n : snap.nodes) {
            auto entity = m_registry.create();
            m_registry.emplace<Position>(entity, n.x, n.y);
            m_registry.emplace<Style>(entity, n.shade, n.radius, n.borderless);
            m_registry.emplace<Label>(entity, n.text, n.pos);
            mapping[n.id] = entity;
        }

        for (const auto& e : snap.edges) {
            auto entity = m_registry.create();
            if (mapping.count(e.sourceId) && mapping.count(e.targetId)) {
                m_registry.emplace<Edge>(entity, mapping[e.sourceId], mapping[e.targetId]);
            }
        }
    
        update();
    }
}

void GraphCanvas::clearWorkspace() {
    m_registry.clear();
    saveSnapshot();
    update();
}

QString GraphCanvas::exportToCSV() const {
    QString csv = "TYPE,id,x,y,label,shade,radius,pos\n";
    m_registry.view<Position, Label, Style>().each([&](auto e, const auto& p, const auto& l, const auto& s) {
        csv += QString("NODE,%1,%2,%3,%4,%5,%6,%7\n").arg((int)e).arg(p.x).arg(p.y).arg(QString::fromStdString(l.text)).arg(s.shade).arg(s.radius).arg((int)l.pos);
    });
    m_registry.view<Edge>().each([&](auto, const auto& edge) {
        csv += QString("EDGE,%1,%2,,,,,\n").arg((int)edge.source).arg((int)edge.target);
    });
    return csv;
}

void GraphCanvas::importFromCSV(const QString& csvString) {
    QStringList lines = csvString.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) return;

    m_registry.clear();
    std::unordered_map<int, entt::entity> mapping;

    for (int i = 1; i < lines.size(); ++i) { // skip header
        QStringList cols = lines[i].split(',');
        if (cols.isEmpty()) continue;
        if (cols[0] == "NODE" && cols.size() >= 8) {
            auto entity = m_registry.create();
            mapping[cols[1].toInt()] = entity;
            m_registry.emplace<Position>(entity, cols[2].toFloat(), cols[3].toFloat());
            m_registry.emplace<Label>(entity, cols[4].toStdString(), static_cast<LabelPosition>(cols[7].toInt()));
            m_registry.emplace<Style>(entity, cols[5].toInt(), cols[6].toFloat());
        } else if (cols[0] == "EDGE" && cols.size() >= 3) {
            int src = cols[1].toInt();
            int tgt = cols[2].toInt();
            if (mapping.count(src) && mapping.count(tgt)) {
                auto edgeEntity = m_registry.create();
                m_registry.emplace<Edge>(edgeEntity, mapping[src], mapping[tgt]);
            }
        }
    }
    
    m_history.clear();
    m_historyIndex = -1;
    saveSnapshot();

    update();
}

void GraphCanvas::importFromJSON(const QString& jsonString) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isNull() || !doc.isObject()) return;
    QJsonObject docObj = doc.object();
    m_registry.clear();

    std::unordered_map<int, entt::entity> mapping;
    QJsonArray nodesArr = docObj["nodes"].toArray();
    for (int i = 0; i < nodesArr.size(); ++i) {
        QJsonObject n = nodesArr[i].toObject();
        auto entity = m_registry.create();
        int oldId = n["id"].toInt();
        mapping[oldId] = entity;
        m_registry.emplace<Position>(entity, static_cast<float>(n["x"].toDouble()), static_cast<float>(n["y"].toDouble()));
        m_registry.emplace<Style>(entity, n["shade"].toInt(), static_cast<float>(n["radius"].toDouble()), n["borderless"].toBool());
        m_registry.emplace<Label>(entity, n["label"].toString().toStdString(), static_cast<LabelPosition>(n["labelPos"].toInt()));
    }

    QJsonArray edgesArr = docObj["edges"].toArray();
    for (int i = 0; i < edgesArr.size(); ++i) {
        QJsonObject o = edgesArr[i].toObject();
        int src = o["source"].toInt();
        int tgt = o["target"].toInt();
        if (mapping.count(src) && mapping.count(tgt)) {
            auto edgeEntity = m_registry.create();
            m_registry.emplace<Edge>(edgeEntity, mapping[src], mapping[tgt]);
        }
    }

    m_history.clear();
    m_historyIndex = -1;
    saveSnapshot();

    update();
}

void GraphCanvas::importFromSVG(const QString& svgString) {
    int startIdx = svgString.indexOf("<!-- GRAPH_DATA_BEGIN");
    int endIdx = svgString.indexOf("GRAPH_DATA_END -->");
    if (startIdx != -1 && endIdx != -1) {
        startIdx += 21; // length of begin block
        QString jsonStr = svgString.mid(startIdx, endIdx - startIdx).trimmed();
        importFromJSON(jsonStr);
    }
}

void GraphCanvas::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPointF pos = mapToScene(event->position());
        entt::entity clickedNode = entt::null;
        m_registry.view<Position, Style>().each([&](auto entity, const auto& p, const auto& s) {
            if (QLineF(QPointF(p.x, p.y), pos).length() < s.radius) {
                clickedNode = entity;
            }
        });

        if (clickedNode != entt::null) {
            m_drawingEdgeSource = clickedNode;
            m_drawingEdgeEnd = pos;
            update();
        }
    }
}

void GraphCanvas::hoverMoveEvent(QHoverEvent* event)
{
    m_lastHoverPos = event->position();
    if (m_drawingEdgeSource != entt::null) {
        m_drawingEdgeEnd = mapToScene(event->position());
        update();
    }
}

void GraphCanvas::pasteExample(int type) {
    float ox = 10.0f, oy = 20.0f; 
    switch(type) {
        case 0: GraphExamples::injectExpander(m_registry, ox, oy); break; // 3-to-2 expander
        case 1: GraphExamples::injectNeuralNet(m_registry, ox, oy); break; // NN
        case 2: GraphExamples::injectBinaryTree(m_registry, ox, oy); break; // BTS
        case 3: GraphExamples::injectK6(m_registry, ox, oy); break; // K6
    }
    saveSnapshot();
    update();
}
