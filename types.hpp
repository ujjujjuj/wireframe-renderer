#include <set>
#include <string>
#include <tuple>
#include <vector>

typedef std::tuple<uint8_t, uint8_t, uint8_t> Color;
typedef std::tuple<float, float, float> Vertex;
typedef std::tuple<uint32_t, uint32_t> Edge;

std::vector<Color> colors = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 0, 255}, {0, 255, 255}};

struct Shape {
    std::string name;
    std::vector<Vertex> vertices;
    std::set<Edge> edges;
    Color color;
    Shape() {
        color = colors[rand() % colors.size()];
    }
};
