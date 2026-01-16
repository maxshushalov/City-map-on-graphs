#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <queue>

#include "Graph.hpp"

const int ROWS_OF_CITY = 6;
const int COLS_OF_CITY = 8;
const int TOTAL_VERTICES = ROWS_OF_CITY * COLS_OF_CITY;

std::mt19937 rng(std::random_device{}());

bool isConnected(const Graph& g) {
    int n = g.getVertexCount();
    if (n == 0) return false;

    std::vector<bool> visited(n, false);
    std::queue<int> q;

    q.push(0);
    visited[0] = true;
    int count = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const auto& edge : g.getNeighbors(u)) {
            int v = edge.to;
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
                count++;
            }
        }
    }
    return count == n;
}

int main() {
    std::cout << "=== CITY GRAPH GENERATOR ===\n";
    std::cout << "Generating " << ROWS_OF_CITY << "x" << COLS_OF_CITY << " = "
              << TOTAL_VERTICES << " vertices\n\n";

    Graph city;
    int coeff = 150;
    int attempt = 0;

    const double EDGE_PROBABILITY = 0.80;      // шанс добавить пару вершин
    const double BASE_DISTANCE    = 50.0;      // км
    const double BASE_TIME        = 5.0;       // минут
    const double BASE_COST        = 50.0;      // руб

    while (true) {
        attempt++;
        std::cout << "--- Attempt #" << attempt << " ---\n";

        city.clear();

        for (int i = 0; i < TOTAL_VERTICES; i++) {
            city.addVertex();

            int row = i / COLS_OF_CITY;
            int col = i % COLS_OF_CITY;
            int x = col * coeff;
            int y = row * coeff;

            city.setVertexCoordinates(i, x, y, "V" + std::to_string(i));
        }

        std::cout << "✓ Created " << TOTAL_VERTICES << " vertices\n";

        struct PotentialEdge {
            int from;
            int to;
        };

        std::vector<PotentialEdge> uniquePairs;

        for (int i = 0; i < TOTAL_VERTICES; i++) {
            int row = i / COLS_OF_CITY;
            int col = i % COLS_OF_CITY;

            if (col < COLS_OF_CITY - 1) {
                int right = i + 1;
                uniquePairs.push_back({i, right}); // только прямое направление
            }

            if (row < ROWS_OF_CITY - 1) {
                int down = i + COLS_OF_CITY;
                uniquePairs.push_back({i, down}); // только прямое направление
            }
        }

        std::cout << "Total unique pairs: " << uniquePairs.size() << "\n";

        std::shuffle(uniquePairs.begin(), uniquePairs.end(), rng);
        std::uniform_real_distribution<double> prob(0.0, 1.0);

        int added = 0;
        int skipped = 0;

        for (const auto& pair : uniquePairs) {
            if (prob(rng) < 0.80) { // 80% вероятность добавить дорогу
                // Качество дороги одинаковое в обоих направлениях
                std::string quality = (prob(rng) < 0.15) ? "bad" : "good";

                city.addEdge(pair.from, Edge(pair.to, 50.0, 5.0, 50.0, quality));
                city.addEdge(pair.to, Edge(pair.from, 50.0, 5.0, 50.0, quality));

                added += 2;
            } else {
                skipped += 2;
            }
        }

        std::cout << "Added edges: " << added << "\n";
        std::cout << "Skipped edges: " << skipped << " (~"
                  << (100 * skipped / (added + skipped)) << "%)\n";


        if (isConnected(city)) {
            std::cout << "✓ Graph is connected!\n\n";
            break;  // Выходим из цикла генерации
        } else {
            std::cout << " Graph is NOT connected. Retrying...\n\n";
        }
    }

    city.exportToJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/graph.json");
    std::cout << "  City graph saved to: web/graph.json\n\n";

    std::cout << "=== STATISTICS ===\n";
    std::cout << "Vertices: " << city.getVertexCount() << "\n";

    int totalEdges = 0;
    int badRoads   = 0;

    for (int i = 0; i < city.getVertexCount(); ++i) {
        for (const auto& edge : city.getNeighbors(i)) {
            totalEdges++;
            if (edge.quality == "bad") {
                badRoads++;
            }
        }
    }

    std::cout << "Edges (directed): " << totalEdges << "\n";
    std::cout << "Average degree: "
              << (1.0 * totalEdges / city.getVertexCount()) << "\n";
    std::cout << "Good roads: " << (totalEdges - badRoads) << " ("
              << (100 * (totalEdges - badRoads) / std::max(1, totalEdges)) << "%)\n";
    std::cout << "Bad roads: " << badRoads << " ("
              << (100 * badRoads / std::max(1, totalEdges)) << "%)\n";

    std::cout << "\n=== DONE ===\n";
    std::cout << "Successfully generated connected graph in "
              << attempt << " attempt(s)!\n";

    return 0;
}
z