#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <iomanip>
#include <fstream>
#include <cmath>
#include "/Users/maxshushalov/CPP_projects/Tiger/Laba_3/include/nlohmann/json.hpp"

using json = nlohmann::json;

struct Edge {
    int to;          // Индекс вершины назначения
    double distance; //  Расстояние (км)
    double time;     //  Время (мин)
    double cost;     //  Стоимость (руб)
    std::string quality = "good";
    bool blocked = false; // пробка
    double trafficLevel = 1.0; // для пробки ( 1.0 - 3.0)

    Edge(int to = 0,
         double distance = 0.0,
         double time = 0.0,
         double cost = 0.0,
         const std::string& quality = "good",
         bool blocked = false,
         double trafficLevel = 1.0)
            : to(to), distance(distance), time(time), cost(cost),
              quality(quality), blocked(blocked), trafficLevel(trafficLevel) {}

    void applyTraffic(double baseTime) {
        if (blocked) {
            time = std::numeric_limits<double>::infinity();
        } else {
            time = baseTime * trafficLevel;
        }
    }

    bool isAvailable() const {
        return !blocked;
    }

};

struct Vertex { // вершина под А*
    int x;
    int y;
    std::string name;
};

class Graph {
public:
    Graph() = default;

    explicit Graph(int vertices);

    void setVertexCoordinates(int id, int x, int y, const std::string& name = "");
    Vertex getVertex(int id) const;
    bool hasCoordinates() const;

    void addVertex();
    void addEdge(int from, const Edge& edge);
    std::vector<Edge>& getNeighbors(int vertex);
    const std::vector<Edge>& getNeighbors(int vertex) const;
    int getVertexCount() const;
    void clear();

    void generateRandom(int vertices, double density);
    void exportToJSON(const std::string& filename) const;
    void loadFromJSON(const std::string& filename) ;

    void print() const;

private:
    int V = 0; // vertices counter
    std::vector<std::vector<Edge>> adjList;
    std::vector<Vertex> vertices; // координаты вершин для а*
};

Graph::Graph(int vertices) {
    V = vertices;
    adjList.resize(V);
}

void Graph::addVertex() {
    V++;
    adjList.resize(V);
}

void Graph::addEdge(int from, const Edge& edge) {
    if (from >= 0 && from < V && edge.to >= 0 && edge.to < V) {
        adjList[from].push_back(edge);
    }
}

std::vector<Edge>& Graph::getNeighbors(int vertex) {
    return adjList.at(vertex);
}

const std::vector<Edge>& Graph::getNeighbors(int vertex) const {
    return adjList.at(vertex);
}

int Graph::getVertexCount() const {
    return V;
}

void Graph::clear() {
    V = 0;
    adjList.clear();
}

void Graph::generateRandom(int vertices, double density) {
    clear();
    V = vertices;
    adjList.resize(V);

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> probDist(0.0, 1.0);

    // Генераторы для весов
    std::uniform_real_distribution<double> distRange(5.0, 100.0); // 5-100 км
    std::uniform_real_distribution<double> speedRange(30.0, 110.0); // км/ч

    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) {
            if (i == j) continue; // Без петель

            if (probDist(rng) < density) {
                double d = distRange(rng);
                double speed = speedRange(rng);

                double t = (d / speed) * 60.0; // время в минутах
                double c = d * 15.0 + (speed > 90 ? 200 : 0); // цена зависит от расстояния и скорости

                std::string quality = (probDist(rng) < 0.15) ? "bad" : "good";
                addEdge(i, Edge(j, d, t, c, quality));
            }
        }
    }
}

void Graph::exportToJSON(const std::string& filename) const {
    json data;

    data["vertices"] = json::array();

    for (int i = 0; i < V; ++i) {
        json vertex;
        vertex["id"] = i;

        if (hasCoordinates()) {
            Vertex v = getVertex(i);
            vertex["name"] = v.name;
            vertex["x"] = v.x;
            vertex["y"] = v.y;
        } else {
            double angle = 2.0 * M_PI * i / V;
            vertex["name"] = "V" + std::to_string(i);
            vertex["x"] = 400 + 150 * std::cos(angle);
            vertex["y"] = 300 + 150 * std::sin(angle);
        }

        data["vertices"].push_back(vertex);
    }

    data["edges"] = json::array();

    for (int i = 0; i < V; ++i) {
        for (const auto& edge : adjList[i]) {
            json edgeData;
            edgeData["from"] = i;
            edgeData["to"] = edge.to;
            edgeData["distance"] = edge.distance;
            edgeData["time"] = edge.time;
            edgeData["cost"] = edge.cost;
            edgeData["quality"] = edge.quality;

            edgeData["blocked"] = edge.blocked;
            edgeData["trafficLevel"] = edge.trafficLevel;

            data["edges"].push_back(edgeData);
        }
    }

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing!\n";
        return;
    }

    outFile << data.dump(2);  // 2 = отступ для красивого форматирования
    outFile.close();

    std::cout << "Graph exported to " << filename << " successfully!\n";
}


void Graph::loadFromJSON(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for reading!\n";
        return;
    }

    json data;
    try {
        inFile >> data;
    } catch (const json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << "\n";
        inFile.close();
        return;
    }
    inFile.close();

    clear();

    if (data.contains("vertices")) {
        for (const auto& vertex : data["vertices"]) {
            int id = vertex["id"];
            std::string name = vertex["name"];
            double x = vertex["x"];
            double y = vertex["y"];

            addVertex();
            setVertexCoordinates(id, x, y, name);
        }
    }

    if (data.contains("edges")) {
        for (const auto& edge : data["edges"]) {
            int from = edge["from"];
            int to = edge["to"];
            double distance = edge["distance"];
            double time = edge["time"];
            double cost = edge["cost"];


            std::string quality = "good";
            if (edge.contains("quality")) {
                quality = edge["quality"];
            }

            bool blocked = false;
            if (edge.contains("blocked")) blocked = edge["blocked"];

            double trafficLevel = 1.0;
            if (edge.contains("trafficLevel")) trafficLevel = edge["trafficLevel"];

            addEdge(from, Edge(to, distance, time, cost, quality, blocked, trafficLevel));
        }
    }

    std::cout << "Graph loaded from " << filename << " successfully!\n";
    std::cout << "Loaded " << V << " vertices and ";

    int edgeCount = 0;
    for (int i = 0; i < V; ++i) {
        edgeCount += adjList[i].size();
    }
    std::cout << edgeCount << " edges.\n";
}


void Graph::print() const {
    std::cout << "=== Graph Adjacency List ===\n";
    for (int i = 0; i < V; ++i) {
        std::cout << "Vertex " << i << ":\n";
        if (adjList[i].empty()) {
            std::cout << "  (no outgoing edges)\n";
        }
        for (const auto& edge : adjList[i]) {
            std::cout << "  -> " << edge.to
                      << " [Dist: " << std::fixed << std::setprecision(1) << edge.distance
                      << ", Time: " << edge.time
                      << ", Cost: " << edge.cost << "]\n";
        }
    }
    std::cout << "============================\n";
}

void Graph::setVertexCoordinates(int id, int x, int y, const std::string &name) {
    if (vertices.size() < static_cast<size_t>(V)) {
        vertices.resize(V);
    }
    if (id >= 0 && id < V) {
        vertices[id] = {x, y, name.empty() ? ("V" + std::to_string(id)) : name};
    }
}

Vertex Graph::getVertex(int id) const {
    if (id>= 0 && id < (int)vertices.size()){
        return vertices[id];
    }
    return {0,0,""};
}

bool Graph::hasCoordinates() const {
    return !vertices.empty();
}