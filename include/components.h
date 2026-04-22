#pragma once
#include <string>
#include <vector>
#include <entt/entt.hpp>
#include <QString>

struct Position { float x = 100.f, y = 100.f; };  // mm
enum class LabelPosition { Center, Top, Bottom, Left, Right };
struct Label { std::string text = ""; LabelPosition pos = LabelPosition::Center; };
struct Edge { entt::entity source; entt::entity target; };
struct Selected {};

struct Style { int shade = 0; float radius = 20.f; bool borderless = false; };

struct CopiedNode { float x, y; int shade; float radius; std::string text; LabelPosition pos; bool borderless; };

struct SnapshotNode { int id; float x, y; int shade; float radius; std::string text; LabelPosition pos; bool borderless; };
struct SnapshotEdge { int sourceId, targetId; };
struct Snapshot { std::vector<SnapshotNode> nodes; std::vector<SnapshotEdge> edges; };
