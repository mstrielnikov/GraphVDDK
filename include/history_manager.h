#pragma once
#include "components.h"
#include <entt/entt.hpp>
#include <vector>

class HistoryManager {
public:
    void saveSnapshot(const entt::registry& reg);
    bool undo(entt::registry& reg);
    bool redo(entt::registry& reg);
    void clear();
    
    int historyIndex() const { return m_historyIndex; }
    size_t historySize() const { return m_history.size(); }

private:
    void restoreSnapshot(entt::registry& reg, const Snapshot& snap);
    
    std::vector<Snapshot> m_history;
    int m_historyIndex = -1;
};
