#pragma once
#include <QQuickPaintedItem>
#include <entt/entt.hpp>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <vector>

struct Position { float x = 100.f, y = 100.f; };  // mm
enum class LabelPosition { Center, Top, Bottom, Left, Right };
struct Label { std::string text = ""; LabelPosition pos = LabelPosition::Center; };
struct Edge { entt::entity source; entt::entity target; };
struct Selected {};

struct CopiedNode { float x, y; int shade; float radius; std::string text; LabelPosition pos; bool borderless; };

struct SnapshotNode { int id; float x, y; int shade; float radius; std::string text; LabelPosition pos; bool borderless; };
struct SnapshotEdge { int sourceId, targetId; };
struct Snapshot { std::vector<SnapshotNode> nodes; std::vector<SnapshotEdge> edges; };
// shade: 0 = black fill (solid), 255 = white fill, mid-values = grey.
// Follows LaTeX TikZ convention: fill=black!<shade>%
struct Style { int shade = 0; float radius = 20.f; bool borderless = false; };

class GraphCanvas : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(float zoom READ zoom NOTIFY zoomChanged)

public:
    explicit GraphCanvas(QQuickItem* parent = nullptr);
    void paint(QPainter* painter) override;

    float zoom() const { return m_zoom; }

    Q_INVOKABLE QString exportToTikZ() const;
    Q_INVOKABLE QString exportToSVG() const;
    Q_INVOKABLE QString exportToJSON() const;
    Q_INVOKABLE QString exportToCSV() const;
    Q_INVOKABLE void exportToPNG(const QString& filePath) const;
    
    Q_INVOKABLE void importFromJSON(const QString& jsonString);
    Q_INVOKABLE void importFromCSV(const QString& csvString);
    Q_INVOKABLE void importFromSVG(const QString& svgString);

    Q_INVOKABLE void pasteExample(int type);
    Q_INVOKABLE void addNode(float x, float y, int shade = 0, float radius = 20.0f);
    Q_INVOKABLE void autoAlign(int type = 0);
    
    Q_INVOKABLE void setNodeText(int entityId, const QString& text);
    Q_INVOKABLE void setNodeLabelPosition(int entityId, int posIndex);
    Q_INVOKABLE int getNodeLabelPosition(int entityId) const;
    Q_INVOKABLE void toggleNodeBorderless(int entityId);
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void setZoomScale(float percent);
    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clearWorkspace();

signals:
    void zoomChanged();
    void nodeRightClicked(int entityId, const QString& currentText, float x, float y);
    void canvasRightClicked(float x, float y);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawGraph(QPainter* painter) const;
    QPointF mapToScene(const QPointF& pos) const;
    void saveSnapshot();

    entt::registry m_registry;
    entt::entity m_dragged = entt::null;
    std::unordered_map<entt::entity, QPointF> m_dragStartPositions;
    QPointF m_dragSceneStart;
    std::vector<Snapshot> m_history;
    int m_historyIndex = -1;
    std::vector<CopiedNode> m_clipboard;

    QPointF m_panInitialStart;
    QPointF m_lastPos;
    
    // Extrinsic zooming overrides
    float m_zoomScaleFactor = 1.0f;
    QPointF m_panExtraOffset;
    QPointF m_lastHoverPos;
    QTimer* m_edgePanTimer = nullptr;
    
    QPointF m_lastScenePos;
    QPointF m_dragOffset;

    entt::entity m_drawingEdgeSource = entt::null;
    QPointF m_drawingEdgeEnd;

    float m_zoom = 1.0f;
    QPointF m_pan;
    bool m_isPanning = false;
    QPointF m_panStart;

    bool m_isBoxSelecting = false;
    QPointF m_boxSelectStart;
    QPointF m_boxSelectEnd;
};