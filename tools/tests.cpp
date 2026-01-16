#include <iostream>
#include <chrono>
#include "Graph.hpp"
#include "Algorithms.hpp"
#include "generate_city.cpp"
#include "../include/CityManager.hpp"


void printPath(const PathResult& res, const std::string& metricName) {
    if (!res.found) {
        std::cout << "Path not found!\n";
        return;
    }

    std::cout << "Shortest path by [" << metricName << "]: " << res.totalWeight << "\nRoute: ";
    for (size_t i = 0; i < res.path.size(); ++i) {
        std::cout << res.path[i] << (i < res.path.size() - 1 ? " -> " : "");
    }
    std::cout << "\n-----------------------------------\n";
}

// Лямбды для разных критериев
auto distFunc = [](const Edge& e) { return e.distance; };
auto timeFunc = [](const Edge& e) { return e.time; };
auto costFunc = [](const Edge& e) { return e.cost; };

int main() {
    std::setlocale(LC_ALL, "");

    int main_generate_city();

    Graph g;
    bool graphGenerated = false;
    int choice;

    while (true) {
        std::cout << "\n=== LAB 3: Shortest Paths & Graph Algorithms ===\n";
        std::cout << "1. Сгенерировать случайный граф\n";
        std::cout << "2. Вывести граф\n";
        std::cout << "3. Find Path (Min Distance)\n";
        std::cout << "4. Find Path (Min Time)\n";
        std::cout << "5. Find Path (Min Cost)\n";
        std::cout << "6. Benchmark Performance\n";
        std::cout << "7. Export to JSON (for visualization)\n";
        std::cout << "8. Generate Manhattan Grid Graph\n";
        std::cout << "9. Compare Dijkstra vs A*\n";
        std::cout << "10. Test JSON load/save\n";
        std::cout << "12. Smart Pathfinding (with events)\n";
        std::cout << "13. Delivery Mode\n";

        std::cout << "0. Exit\n";
        std::cout << "> ";
        std::cin >> choice;

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                int v;
                std::cout << "Enter number of vertices: ";
                std::cin >> v;
                // Плотность 20% по дефолту
                g.generateRandom(v, 0.2);
                graphGenerated = true;
                std::cout << "Graph generated successfully with " << v << " vertices.\n";
                break;
            }
            case 2: {
                if (!graphGenerated) std::cout << "Generate graph first!\n";
                else g.print();
                break;
            }
            case 3:
            case 4:
            case 5: {
                if (!graphGenerated) {
                    std::cout << "Generate graph first!\n";
                    break;
                }
                int start, end;
                std::cout << "Enter Start and End vertices (0 - " << g.getVertexCount() - 1 << "): ";
                std::cin >> start >> end;

                PathResult res;
                if (choice == 3) {
                    res = Algorithms::dijkstra(g, start, end, distFunc);
                    printPath(res, "Distance");
                } else if (choice == 4) {
                    res = Algorithms::dijkstra(g, start, end, timeFunc);
                    printPath(res, "Time");
                } else {
                    res = Algorithms::dijkstra(g, start, end, costFunc);
                    printPath(res, "Cost");
                }
                break;
            }
            case 6: {
                if (!graphGenerated) {
                    std::cout << "Generate graph first! (Preferably > 500 vertices for test)\n";
                    break;
                }
                std::cout << "Running benchmark (finding paths 0 -> last)...\n";
                int target = g.getVertexCount() - 1;

                auto start = std::chrono::high_resolution_clock::now();
                Algorithms::dijkstra(g, 0, target, distFunc);
                auto end = std::chrono::high_resolution_clock::now();

                std::chrono::duration<double, std::milli> elapsed = end - start;
                std::cout << "Dijkstra execution time: " << elapsed.count() << " ms\n";
                break;
            }

            case 7: {
                if (!graphGenerated) {
                    std::cout << "Generate graph first!\n";
                    break;
                }
                g.exportToJSON("graph.json");
                std::cout << "Now copy graph.json to web/ folder and open browser\n";
                break;
            }
            case 8: {
                int gridSize;
                std::cout << "Enter grid size (e.g., 5 for 5x5 = 25 vertices): ";
                std::cin >> gridSize;

                // Генератор случайных чисел для качества дорог
                std::random_device rd;
                std::mt19937 rng(rd());
                std::uniform_real_distribution<double> prob(0.0, 1.0);

                int totalVertices = gridSize * gridSize;
                g.clear();

                for (int i = 0; i < totalVertices; ++i) {
                    g.addVertex();
                    int row = i / gridSize;
                    int col = i % gridSize;
                    g.setVertexCoordinates(i, col * 50, row * 50, "V" + std::to_string(i));
                }

                for (int i = 0; i < totalVertices; ++i) {
                    int row = i / gridSize;
                    int col = i % gridSize;

                    // Сосед справа
                    if (col < gridSize - 1) {
                        int right = i + 1;
                        std::string quality = (prob(rng) < 0.15) ? "bad" : "good";

                        g.addEdge(i, Edge(right, 50, 10, 50, quality));
                        g.addEdge(right, Edge(i, 50, 10, 50, quality));
                    }

                    if (row < gridSize - 1) {
                        int down = i + gridSize;
                        std::string quality = (prob(rng) < 0.15) ? "bad" : "good";

                        g.addEdge(i, Edge(down, 50, 10, 50, quality));
                        g.addEdge(down, Edge(i, 50, 10, 50, quality));
                    }
                }

                graphGenerated = true;
                std::cout << "Manhattan grid " << gridSize << "x" << gridSize
                          << " (" << totalVertices << " vertices) generated!\n";
                break;
            }

            case 9 : {
                if (!graphGenerated){
                    std::cout<< "Generate graph first!\n";
                    break;
                }

                if (!g.hasCoordinates()){
                    std::cerr << "Graph has no coordinates! Use option 8 to generate Manhattan grid.\n";
                    break;
                }

                int start, end;

                std::cout << "Enter Strart and End verticies: ";
                std::cin >> start >> end;

                // Dijkstra
                auto t1 = std::chrono::high_resolution_clock::now();
                PathResult resDijkstra = Algorithms::dijkstra(g, start, end, distFunc);
                auto t2 = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> timeDijkstra = t2 - t1;

                // A*
                auto t3 = std::chrono::high_resolution_clock::now();
                PathResult resAStar = Algorithms::aStar(g, start, end, distFunc);
                auto t4 = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> timeAStar = t4 - t3;

                // Результаты
                std::cout << "\n=== COMPARISON ===\n";
                std::cout << "Dijkstra:\n";
                printPath(resDijkstra, "Distance");
                std::cout << "Time: " << timeDijkstra.count() << " ms\n\n";

                std::cout << "A* (Manhattan heuristic):\n";
                printPath(resAStar, "Distance");
                std::cout << "Time: " << timeAStar.count() << " ms\n\n";

                if (resDijkstra.totalWeight == resAStar.totalWeight) {
                    std::cout << "✓ Both found optimal path!\n";
                }
                std::cout << "Speedup: " << (timeDijkstra.count() / timeAStar.count()) << "x\n";
                break;
            }
            case 10 : {
                std::cout << "\n=== TEST: JSON Load/Save ===\n";

                Graph city;
                city.loadFromJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/data/city_graph.json");

                std::cout << "\n--- First 3 vertices ---\n";
                for (int i = 0; i < 3; ++i) {
                    Vertex v = city.getVertex(i);
                    std::cout << v.name << " at (" << v.x << ", " << v.y << ")\n";
                    std::cout << "  Edges: ";
                    for (const auto& e : city.getNeighbors(i)) {
                        std::cout << e.to << " ";
                    }
                    std::cout << "\n";
                }

                city.exportToJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/data/city_test.json");
                std::cout << "\n✓ Test completed!\n";
                break;
            }
            case 11: {
                std::cout << "\n=== TEST: CityManager ===\n";

                Graph city;
                city.loadFromJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/data/city_graph.json");

                CityManager manager;

                std::cout << "\n--- Generating warehouses ---\n";
                manager.generateWarehouses(city, 5);

                std::cout << "\n--- Hour 18 (rush hour) ---\n";
                manager.generateEvents(city, 18);
                manager.printEventStats(city);

                std::cout << "\n--- Hour 3 (night) ---\n";
                manager.generateEvents(city, 3);
                manager.printEventStats(city);

                std::cout << "\n✓ CityManager test completed!\n";
                break;
            }
            case 12: { // Умный поиск пути
                std::cout << "\n=== SMART PATHFINDING ===\n";

                Graph city;
                city.loadFromJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/data/city_graph.json");

                CityManager manager;
                manager.generateWarehouses(city, 5);

                int hour, destination;
                char fragileInput;

                std::cout << "Время суток (0-23): ";
                std::cin >> hour;

                std::cout << "Пункт назначения: ";
                std::cin >> destination;

                std::cout << "Хрупкий груз? (y/n): ";
                std::cin >> fragileInput;
                bool fragile = (fragileInput == 'y' || fragileInput == 'Y');

                manager.generateEvents(city, hour);

                auto smartFunc = [fragile](const Edge& e) -> double {
                    if (e.blocked) return std::numeric_limits<double>::infinity();
                    double time = e.time * e.trafficLevel;
                    if (fragile && e.quality == "bad") time *= 10;
                    return time;
                };

                PathResult result = Algorithms::dijkstra(city, manager.getMainWarehouse(), destination, smartFunc);

                if (result.found) {
                    std::cout << "\n✓ Маршрут найден!\n";
                    std::cout << "Путь: ";
                    for (size_t i = 0; i < result.path.size(); ++i) {
                        std::cout << result.path[i];
                        if (i < result.path.size() - 1) std::cout << " → ";
                    }
                    std::cout << "\nВремя: " << result.totalWeight << " мин\n";
                } else {
                    std::cout << "\n✗ Путь не найден\n";
                }

                break;
            }

            case 13: { // Режим доставки
                std::cout << "\n=== DELIVERY MODE ===\n";

                Graph city;
                city.loadFromJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/data/city_graph.json");

                CityManager manager;
                manager.generateWarehouses(city, 5);

                int hour;
                char fragileInput;

                std::cout << "Время суток (0-23): ";
                std::cin >> hour;

                std::cout << "Хрупкий груз? (y/n): ";
                std::cin >> fragileInput;
                bool fragile = (fragileInput == 'y' || fragileInput == 'Y');

                manager.generateEvents(city, hour);

                auto smartFunc = [fragile](const Edge& e) -> double {
                    if (e.blocked) return std::numeric_limits<double>::infinity();
                    double time = e.time * e.trafficLevel;
                    if (fragile && e.quality == "bad") time *= 10;
                    return time;
                };

                int mainWH = manager.getMainWarehouse();
                double totalTime = 0.0;
                int successCount = 0;

                std::cout << "\n=== МАРШРУТЫ ДОСТАВКИ ===\n";
                for (int deliveryWH : manager.getDeliveryWarehouses()) {
                    std::cout << "\nМаршрут: " << mainWH << " → " << deliveryWH << "\n";

                    PathResult result = Algorithms::dijkstra(city, mainWH, deliveryWH, smartFunc);

                    if (result.found) {
                        std::cout << "  ✓ Путь: ";
                        for (size_t i = 0; i < result.path.size(); ++i) {
                            std::cout << result.path[i];
                            if (i < result.path.size() - 1) std::cout << " → ";
                        }
                        std::cout << "\n  Время: " << result.totalWeight << " мин\n";
                        totalTime += result.totalWeight;
                        successCount++;
                    } else {
                        std::cout << "  ✗ Путь не найден\n";
                    }
                }

                std::cout << "\n=== ИТОГИ ===\n";
                std::cout << "Успешных доставок: " << successCount << " / "
                          << manager.getDeliveryWarehouses().size() << "\n";
                std::cout << "Общее время: " << totalTime << " мин\n";

                break;
            }

            default:
                std::cout << "Invalid option.\n";
        }
    }

    return 0;
}
