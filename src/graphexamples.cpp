#include "graphexamples.h"
#include "graphcanvas.h"
#include <cmath>

namespace GraphExamples {

void injectExpander(entt::registry& reg, float offsetX, float offsetY) {
    // 3-to-2 expander graph (bipartite: L=4, R=6)
    std::vector<entt::entity> L, R;
    for(int i=0; i<4; i++) {
        auto n = reg.create();
        reg.emplace<Position>(n, offsetX + 30.f, offsetY + 30.f + i * 40.f);
        reg.emplace<Style>(n, 0, 15.f);
        reg.emplace<Label>(n, QString("L%1").arg(i).toStdString(), LabelPosition::Center);
        L.push_back(n);
    }
    for(int i=0; i<6; i++) {
        auto n = reg.create();
        reg.emplace<Position>(n, offsetX + 150.f, offsetY + 0.f + i * 30.f);
        reg.emplace<Style>(n, 120, 15.f);
        reg.emplace<Label>(n, QString("R%1").arg(i).toStdString(), LabelPosition::Center);
        R.push_back(n);
    }
    auto addEdge = [&](int l, int r) {
        auto e = reg.create();
        reg.emplace<Edge>(e, L[l], R[r]);
    };
    addEdge(0, 0); addEdge(0, 1); addEdge(0, 2);
    addEdge(1, 2); addEdge(1, 3); addEdge(1, 4);
    addEdge(2, 4); addEdge(2, 5); addEdge(2, 0);
    addEdge(3, 1); addEdge(3, 3); addEdge(3, 5);
}

void injectNeuralNet(entt::registry& reg, float offsetX, float offsetY) {
    std::vector<entt::entity> L1, L2, L3;
    for(int i=0; i<3; i++) {
        auto n = reg.create();
        reg.emplace<Position>(n, offsetX + 20.f, offsetY + 50.f + i * 40.f);
        reg.emplace<Style>(n, 0, 15.f);
        reg.emplace<Label>(n, "", LabelPosition::Center);
        L1.push_back(n);
    }
    for(int i=0; i<5; i++) { 
        auto n = reg.create();
        reg.emplace<Position>(n, offsetX + 100.f, offsetY + 10.f + i * 40.f);
        reg.emplace<Style>(n, 100, 15.f);
        reg.emplace<Label>(n, "", LabelPosition::Center);
        L2.push_back(n);
    }
    for(int i=0; i<2; i++) {
        auto n = reg.create();
        reg.emplace<Position>(n, offsetX + 180.f, offsetY + 70.f + i * 40.f);
        reg.emplace<Style>(n, 200, 15.f);
        reg.emplace<Label>(n, "", LabelPosition::Center);
        L3.push_back(n);
    }
    for(auto n1 : L1) for(auto n2 : L2) {
        auto e = reg.create(); reg.emplace<Edge>(e, n1, n2);
    }
    for(auto n2 : L2) for(auto n3 : L3) {
        auto e = reg.create(); reg.emplace<Edge>(e, n2, n3);
    }
}

void injectBinaryTree(entt::registry& reg, float offsetX, float offsetY) {
    // Binary tree height 4 (15 nodes)
    std::vector<entt::entity> nodes;
    for(int i=0; i<15; i++) {
        int level = std::log2(i + 1);
        int cols = 1 << level;
        int col = (i + 1) - cols;
        float spacing = 180.0f / cols;
        float x = offsetX + 15.0f + (col + 0.5f) * spacing;
        float y = offsetY + 20.f + level * 50.f;
        auto n = reg.create();
        reg.emplace<Position>(n, x, y);
        reg.emplace<Style>(n, level * 50, 15.f);
        reg.emplace<Label>(n, "", LabelPosition::Center);
        nodes.push_back(n);
        if (i > 0) {
            int parent = (i - 1) / 2;
            auto e = reg.create();
            reg.emplace<Edge>(e, nodes[parent], nodes[i]);
        }
    }
}

void injectK6(entt::registry& reg, float offsetX, float offsetY) {
    std::vector<entt::entity> nodes;
    float cx = offsetX + 105.f;
    float cy = offsetY + 100.f;
    float r = 70.0f;
    for(int i=0; i<6; i++) {
        float angle = i * static_cast<float>(M_PI) / 3.0f;
        auto n = reg.create();
        reg.emplace<Position>(n, 
            cx + r * std::cos(angle), 
            cy + r * std::sin(angle)
        );
        reg.emplace<Style>(n, i * 40, 15.f);
        reg.emplace<Label>(n, "", LabelPosition::Center);
        nodes.push_back(n);
    }
    for(int i=0; i<6; i++) {
        for(int j=i+1; j<6; j++) {
            auto e = reg.create();
            reg.emplace<Edge>(e, nodes[i], nodes[j]);
        }
    }
}

} // namespace GraphExamples
