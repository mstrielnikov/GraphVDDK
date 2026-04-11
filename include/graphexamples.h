#pragma once
#include <entt/entt.hpp>
#include <vector>

namespace GraphExamples {
    void injectExpander(entt::registry& reg, float offsetX, float offsetY);
    void injectNeuralNet(entt::registry& reg, float offsetX, float offsetY);
    void injectBinaryTree(entt::registry& reg, float offsetX, float offsetY);
    void injectK6(entt::registry& reg, float offsetX, float offsetY);
}
