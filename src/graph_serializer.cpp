#include "graph_serializer.h"
#include "components.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

QString GraphSerializer::toJSON(const entt::registry& reg) {
    QJsonArray nodesArr;
    reg.view<Position, Label, Style>().each([&](auto e, const auto& p, const auto& l, const auto& s) {
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
    reg.view<Edge>().each([&](auto /*e*/, const auto& edge) {
        QJsonObject o; o["source"] = (int)edge.source; o["target"] = (int)edge.target;
        edgesArr.append(o);
    });
    QJsonObject docObj;
    docObj["nodes"] = nodesArr;
    docObj["edges"] = edgesArr;
    QJsonDocument doc(docObj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString GraphSerializer::toCSV(const entt::registry& reg) {
    QString csv = "TYPE,id,x,y,label,shade,radius,pos\n";
    reg.view<Position, Label, Style>().each([&](auto e, const auto& p, const auto& l, const auto& s) {
        csv += QString("NODE,%1,%2,%3,%4,%5,%6,%7\n").arg((int)e).arg(p.x).arg(p.y).arg(QString::fromStdString(l.text)).arg(s.shade).arg(s.radius).arg((int)l.pos);
    });
    reg.view<Edge>().each([&](auto, const auto& edge) {
        csv += QString("EDGE,%1,%2,,,,,\n").arg((int)edge.source).arg((int)edge.target);
    });
    return csv;
}

void GraphSerializer::fromJSON(entt::registry& reg, const QString& jsonString) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isNull() || !doc.isObject()) return;
    QJsonObject docObj = doc.object();
    reg.clear();

    std::unordered_map<int, entt::entity> mapping;
    QJsonArray nodesArr = docObj["nodes"].toArray();
    for (int i = 0; i < nodesArr.size(); ++i) {
        QJsonObject n = nodesArr[i].toObject();
        auto entity = reg.create();
        int oldId = n["id"].toInt();
        mapping[oldId] = entity;
        reg.emplace<Position>(entity, static_cast<float>(n["x"].toDouble()), static_cast<float>(n["y"].toDouble()));
        reg.emplace<Style>(entity, n["shade"].toInt(), static_cast<float>(n["radius"].toDouble()), n["borderless"].toBool());
        reg.emplace<Label>(entity, n["label"].toString().toStdString(), static_cast<LabelPosition>(n["labelPos"].toInt()));
    }

    QJsonArray edgesArr = docObj["edges"].toArray();
    for (int i = 0; i < edgesArr.size(); ++i) {
        QJsonObject o = edgesArr[i].toObject();
        int src = o["source"].toInt();
        int tgt = o["target"].toInt();
        if (mapping.count(src) && mapping.count(tgt)) {
            auto edgeEntity = reg.create();
            reg.emplace<Edge>(edgeEntity, mapping[src], mapping[tgt]);
        }
    }
}

void GraphSerializer::fromCSV(entt::registry& reg, const QString& csvString) {
    QStringList lines = csvString.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) return;

    reg.clear();
    std::unordered_map<int, entt::entity> mapping;

    for (int i = 1; i < lines.size(); ++i) { // skip header
        QStringList cols = lines[i].split(',');
        if (cols.isEmpty()) continue;
        if (cols[0] == "NODE" && cols.size() >= 8) {
            auto entity = reg.create();
            mapping[cols[1].toInt()] = entity;
            reg.emplace<Position>(entity, cols[2].toFloat(), cols[3].toFloat());
            reg.emplace<Label>(entity, cols[4].toStdString(), static_cast<LabelPosition>(cols[7].toInt()));
            reg.emplace<Style>(entity, cols[5].toInt(), cols[6].toFloat());
        } else if (cols[0] == "EDGE" && cols.size() >= 3) {
            int src = cols[1].toInt();
            int tgt = cols[2].toInt();
            if (mapping.count(src) && mapping.count(tgt)) {
                auto edgeEntity = reg.create();
                reg.emplace<Edge>(edgeEntity, mapping[src], mapping[tgt]);
            }
        }
    }
}

QString GraphSerializer::extractJSONFromSVG(const QString& svgString) {
    int startIdx = svgString.indexOf("<!-- GRAPH_DATA_BEGIN");
    int endIdx = svgString.indexOf("GRAPH_DATA_END -->");
    if (startIdx != -1 && endIdx != -1) {
        startIdx += 21; // length of begin block
        return svgString.mid(startIdx, endIdx - startIdx).trimmed();
    }
    return QString();
}
