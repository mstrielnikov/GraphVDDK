#include "tikz_exporter.h"
#include <sstream>

std::string TikZExporter::exportToTikZ(const entt::registry& reg)
{
    std::ostringstream tikz;
    tikz << "\\begin{tikzpicture}[scale=1, every node/.style={circle,draw}]\n";

    reg.view<Position, Label, Style>().each([&](auto, const auto& pos, const auto& label, const auto& style) {
        int percentage = 100 - (style.shade * 100 / 255);
        if (style.borderless) {
            tikz << "\\node[draw=none, fill=none";
        } else if (percentage > 0) {
            tikz << "\\node[fill=black!" << percentage;
        } else {
            tikz << "\\node[fill=white";
        }
        
        if (label.pos != LabelPosition::Center && !label.text.empty()) {
            std::string dir = "";
            if (label.pos == LabelPosition::Top) dir = "above";
            else if (label.pos == LabelPosition::Bottom) dir = "below";
            else if (label.pos == LabelPosition::Left) dir = "left";
            else if (label.pos == LabelPosition::Right) dir = "right";
            tikz << ", label=" << dir << ":{" << label.text << "}";
        }

        tikz << "] at (" << pos.x/10.0 << "cm," << -pos.y/10.0 << "cm) {";
        if (label.pos == LabelPosition::Center && !label.text.empty()) {
            tikz << (percentage > 50 ? "\\color{white}" : "\\color{black}");
            tikz << label.text;
        }
        tikz << "};\n";
    });

    // Add edges later
    tikz << "\\end{tikzpicture}\n";
    return tikz.str();
}
