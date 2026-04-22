#include "history_manager.h"
#include <unordered_map>

void HistoryManager::saveSnapshot(const entt::registry& reg) {
    Snapshot snap;
    reg.view<Position, Label, Style>().each([&](auto e, const auto& p, const auto& l, const auto& s) {
        snap.nodes.push_back({(int)e, p.x, p.y, s.shade, s.radius, l.text, l.pos, s.borderless});
    });
    reg.view<Edge>().each([&](auto, const auto& edge) {
        snap.edges.push_back({(int)edge.source, (int)edge.target});
    });

    if (m_historyIndex >= 0 && m_historyIndex < (int)m_history.size() - 1) {
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }

    m_history.push_back(snap);
    m_historyIndex = m_history.size() - 1;
}

bool HistoryManager::undo(entt::registry& reg) {
    if (m_historyIndex > 0) {
        m_historyIndex--;
        restoreSnapshot(reg, m_history[m_historyIndex]);
        return true;
    }
    return false;
}

bool HistoryManager::redo(entt::registry& reg) {
    if (m_historyIndex < static_cast<int>(m_history.size()) - 1) {
        m_historyIndex++;
        restoreSnapshot(reg, m_history[m_historyIndex]);
        return true;
    }
    return false;
}

void HistoryManager::clear() {
    m_history.clear();
    m_historyIndex = -1;
}

void HistoryManager::restoreSnapshot(entt::registry& reg, const Snapshot& snap) {
    reg.clear();
    std::unordered_map<int, entt::entity> mapping;

    for (const auto& n : snap.nodes) {
        auto entity = reg.create();
        reg.emplace<Position>(entity, n.x, n.y);
        reg.emplace<Style>(entity, n.shade, n.radius, n.borderless);
        reg.emplace<Label>(entity, n.text, n.pos);
        mapping[n.id] = entity;
    }

    for (const auto& e : snap.edges) {
        if (mapping.count(e.sourceId) && mapping.count(e.targetId)) {
            auto entity = reg.create();
            reg.emplace<Edge>(entity, mapping[e.sourceId], mapping[e.targetId]);
        }
    }
}
