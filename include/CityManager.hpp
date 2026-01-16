#pragma once
#include "Graph.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

class CityManager {
public:
    CityManager() = default;

    void generateWarehouses(Graph& city, int numDelivery = 5) {
        int V = city.getVertexCount();
        if (V == 0) return;

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, V - 1);

        mainWarehouse = dist(rng);

        deliveryWarehouses.clear();
        while (deliveryWarehouses.size() < static_cast<size_t>(numDelivery)) {
            int wh = dist(rng);
            if (wh != mainWarehouse &&
                std::find(deliveryWarehouses.begin(), deliveryWarehouses.end(), wh)
                == deliveryWarehouses.end()) {
                deliveryWarehouses.push_back(wh);
            }
        }
    }


    int getMainWarehouse() const { return mainWarehouse; }
    const std::vector<int>& getDeliveryWarehouses() const { return deliveryWarehouses; }

    void setMainWarehouse(int vertexId) {
        removeDeliveryWarehouse(vertexId);
        mainWarehouse = vertexId;
    }

    void generateEvents(Graph& city, int hour) {
        clearEvents(city);

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_real_distribution<double> prob(0.0, 1.0);

        int V = city.getVertexCount();

        // Часы пик: 8-10 и 17-19
        bool isRushHour = (hour >= 8 && hour <= 10) || (hour >= 17 && hour <= 19);

        double trafficProb = isRushHour ? 0.30 : 0.10;
        double accidentProb = 0.02;

        int trafficCount = 0;
        int accidentCount = 0;

        for (int i = 0; i < V; ++i) {
            auto& neighbors = city.getNeighbors(i);

            for (auto& edge : neighbors) {
                if (prob(rng) < trafficProb) {
                    edge.trafficLevel = 1.5 + prob(rng) * 1.5;  // 1.5-3.0
                    trafficCount++;
                }
                if (prob(rng) < accidentProb) {
                    edge.blocked = true;
                    accidentCount++;

                    auto& reverseNeighbors = city.getNeighbors(edge.to);
                    for (auto& revEdge : reverseNeighbors) {
                        if (revEdge.to == i) {
                            revEdge.blocked = true;
                            break;
                        }
                    }
                }
            }
        }

    }

    void clearEvents(Graph& city) {
        int V = city.getVertexCount();
        for (int i = 0; i < V; ++i) {
            auto& neighbors = city.getNeighbors(i);
            for (auto& edge : neighbors) {
                edge.blocked = false;
                edge.trafficLevel = 1.0;
            }
        }
    }

    void printEventStats(const Graph& city) const {
        int totalEdges = 0;
        int blockedCount = 0;
        int trafficCount = 0;

        int V = city.getVertexCount();
        for (int i = 0; i < V; ++i) {
            for (const auto& edge : city.getNeighbors(i)) {
                totalEdges++;
                if (edge.blocked) blockedCount++;
                if (edge.trafficLevel > 1.0) trafficCount++;
            }
        }

        std::cout << "\n=== EVENT STATISTICS ===\n";
        std::cout << "Total edges: " << totalEdges << "\n";
        std::cout << "Accidents: " << blockedCount
                  << " (" << (100.0 * blockedCount / totalEdges) << "%)\n";
        std::cout << "Traffic jams: " << trafficCount
                  << " (" << (100.0 * trafficCount / totalEdges) << "%)\n";
    }

    void addDeliveryWarehouse(int vertexId) {
        if (vertexId != mainWarehouse &&
            std::find(deliveryWarehouses.begin(), deliveryWarehouses.end(), vertexId)
            == deliveryWarehouses.end()) {
            deliveryWarehouses.push_back(vertexId);
        }
    }

    void removeDeliveryWarehouse(int vertexId) {
        auto it = std::find(deliveryWarehouses.begin(), deliveryWarehouses.end(), vertexId);
        if (it != deliveryWarehouses.end()) {
            deliveryWarehouses.erase(it);
        }
    }

    void toggleTraffic(Graph& city, int from, int to) {
        auto& neighbors = city.getNeighbors(from);
        for (auto& edge : neighbors) {
            if (edge.to == to) {
                edge.trafficLevel = (edge.trafficLevel > 1.0) ? 1.0 : 2.5;
                break;
            }
        }
    }

    void toggleBlock(Graph& city, int from, int to) {
        auto& neighbors = city.getNeighbors(from);
        for (auto& edge : neighbors) {
            if (edge.to == to) {
                edge.blocked = !edge.blocked;
                break;
            }
        }
    }

    json exportWarehouses() const {
        json data;
        data["mainWarehouse"] = mainWarehouse;
        data["deliveryWarehouses"] = deliveryWarehouses;
        return data;
    }

private:
    int mainWarehouse = -1;
    std::vector<int> deliveryWarehouses;
};
