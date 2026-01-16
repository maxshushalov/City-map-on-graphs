#pragma once
#include "Graph.hpp"
#include "PriorityQueue.hpp"
#include <limits>
#include <functional>
#include <vector>
#include <algorithm>

struct PathResult {
    std::vector<int> path; // Последовательность вершин
    double totalWeight;    // Суммарный вес
    bool found;            // Найден ли путь
};

struct NodeState {
    int vertex;
    double weight;
};

struct AStarNode{
    int vertex;
    double gCost; // реальная стоимость
    double fCost; // g + h
};

struct AStarComporator{
    bool operator()(const AStarNode& a, const AStarNode& b) const {
        return a.fCost > b.fCost; // min-heap по критерию f-cost
    }
};


struct MinHeapComparator {
    bool operator()(const NodeState& a, const NodeState& b) const {
        return a.weight > b.weight;
    }
};

class Algorithms {
public:
    template <typename WeightFunc>
    static PathResult dijkstra(const Graph& graph, int start, int end, WeightFunc getWeight);

    template<typename WeightFunc>
    static PathResult aStar (const Graph& graph, int start, int end, WeightFunc getWeight);
};

template <typename WeightFunc>
PathResult Algorithms::dijkstra(const Graph& graph, int start, int end, WeightFunc getWeight) {
    int n = graph.getVertexCount();

    if (start < 0 || start >= n || end < 0 || end >= n) {
        return {{}, 0.0, false};
    }

    std::vector<double> dist(n, std::numeric_limits<double>::infinity());
    std::vector<int> parent(n, -1);

    PriorityQueue<NodeState, MinHeapComparator> pq;

    dist[start] = 0.0;
    pq.push({start, 0.0});

    while (!pq.empty()) {
        NodeState current = pq.top();
        pq.pop();

        int u = current.vertex;
        double d = current.weight;

        if (d > dist[u]) continue;

        if (u == end) break;

        for (const auto& edge : graph.getNeighbors(u)) {
            int v = edge.to;
            double weight = getWeight(edge);

            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.push({v, dist[v]});
            }
        }
    }

    if (dist[end] == std::numeric_limits<double>::infinity()) {
        return {{}, 0.0, false};
    }

    std::vector<int> path;
    for (int v = end; v != -1; v = parent[v]) {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());

    return {path, dist[end], true};
}

template <typename WeightFunc>
PathResult Algorithms::aStar(const Graph &graph, int start, int end, WeightFunc getWeight) {
    int n = graph.getVertexCount();
    if (start < 0 || start >= n || end < 0 || end >= n){
        return {{}, 0.0, false};
    }

    if (!graph.hasCoordinates()){
        std::cerr << "\"Error: Graph has no coordinates\n";
        return {{}, 0.0, false};
    }

    auto manhattanHeuristic = [&](int nodeId) -> double {
        Vertex current = graph.getVertex(nodeId);
        Vertex goal = graph.getVertex(end);
        return std::abs(goal.x - current.x) + std::abs(goal.y - current.y);
    };

    std::vector<double> gCost (n, std::numeric_limits<double>::infinity());

    std::vector<int> parent (n, -1); // для восстановления пути
    PriorityQueue<AStarNode, AStarComporator> openSet;

    gCost[start] = 0.0;
    double fStart = manhattanHeuristic(start);
    openSet.push({start, 0.0, });

    while (!openSet.empty()){
        AStarNode current = openSet.top();
        openSet.pop();

        int u = current.vertex;

        if (current.gCost > gCost[u]) continue;

        if (u == end) break;

        for (const auto& edge : graph.getNeighbors(u)){
            int v = edge.to;
            double edgeWeight = getWeight(edge);
            double tentativeGCost = gCost[u] + edgeWeight;

            if (tentativeGCost < gCost[v]) {
                gCost[v] = tentativeGCost;
                parent[v] = u;
                double fCost = gCost[v] + manhattanHeuristic(v);
                openSet.push({v, gCost[v], fCost});
            }
        }
    }

    if (gCost[end] == std::numeric_limits<double>::infinity()){
        return {{}, 0.0, false};
    }

    std::vector<int> path;
    for (int v = end; v!= -1; v = parent[v]){
        path.push_back(v);
    }

    std::reverse(path.begin(), path.end());

    return {path, gCost[end], true};

}