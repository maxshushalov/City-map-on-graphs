#include <iostream>
#include <string>
#include <sstream>
#include "Graph.hpp"
#include "Algorithms.hpp"
#include "CityManager.hpp"

using json = nlohmann::json;

static json graphToJson(const Graph& city) {
    json data;
    data["vertices"] = json::array();
    for (int i = 0; i < city.getVertexCount(); ++i) {
        Vertex v = city.getVertex(i);
        data["vertices"].push_back({
           {"id", i},
           {"name", v.name},
           {"x", v.x},
           {"y", v.y}
                                   });
    }

    data["edges"] = json::array();
    for (int from = 0; from < city.getVertexCount(); ++from) {
        for (const auto& e : city.getNeighbors(from)) {
            data["edges"].push_back({
                {"from", from},
                {"to", e.to},
                {"distance", e.distance},
                {"time", e.time},
                {"cost", e.cost},
                {"quality", e.quality},
                {"blocked", e.blocked},
                {"trafficLevel", e.trafficLevel}
                                    });
        }
    }
    return data;
}

static void saveWarehouses(const CityManager& manager) {
    json data = manager.exportWarehouses();
    std::ofstream out("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/warehouses.json");
    if (out.is_open()) {
        out << data.dump(2);
    }
}

static void loadOrInitWarehouses(CityManager& manager, Graph& city) {
    std::ifstream in("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/warehouses.json");
    if (in.is_open()) {
        json data;
        in >> data;
        int mainWh = data.value("mainWarehouse", -1);
        auto delWh = data.value("deliveryWarehouses", std::vector<int>{});

        if (mainWh >= 0 && mainWh < city.getVertexCount()) {
            manager.setMainWarehouse(mainWh);
        }
        for (int id : delWh) {
            if (id >= 0 && id < city.getVertexCount()) {
                manager.addDeliveryWarehouse(id);
            }
        }
        return;
    }

    json emptyData;
    emptyData["mainWarehouse"] = -1;
    emptyData["deliveryWarehouses"] = json::array();

    std::ofstream out("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/warehouses.json");
    if (out.is_open()) {
        out << emptyData.dump(2);
    }
}





int main(int argc, char* argv[]) {

    std::streambuf* oldCout = std::cout.rdbuf(); // старые заглушки, боюсь убирать
    std::ostringstream muted;
    std::cout.rdbuf(muted.rdbuf());

    // Формат парсинга: ./api_pathfinder action=find start=0 end=50 hour=18 fragile=true

    std::map<std::string, std::string> params;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
            std::string key = arg.substr(0, pos);
            std::string value = arg.substr(pos + 1);
            params[key] = value;
        }
    }

    std::string action = params.count("action") ? params["action"] : "find";


    // -- загружаем город --------------------------------------------------------------------
    Graph city;
    city.loadFromJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/graph.json");

    CityManager manager;
    loadOrInitWarehouses(manager, city);


    json response;


    if (action == "generate") {
        int hour = std::stoi(params.count("hour") ? params["hour"] : "9");
        manager.generateEvents(city, hour);

        city.exportToJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/graph.json");

        response["status"] = "success";
        response["message"] = "City generated with events";
    }

    else if (action == "find") {
        int start = std::stoi(params.count("start") ? params["start"] : "0");
        int end = std::stoi(params.count("end") ? params["end"] : "0");

        auto smartFunc = [](const Edge& e) -> double {
            if (e.blocked) return std::numeric_limits<double>::infinity();
            double time = e.time * e.trafficLevel;
            return time;
        };


        PathResult result = Algorithms::dijkstra(city, start, end, smartFunc);

        response["found"]     = result.found;
        response["path"]      = result.path;
        response["totalTime"] = result.totalWeight;

        json edgesInfo = json::array();
        if (result.found && result.path.size() >= 2) {
            for (size_t i = 0; i + 1 < result.path.size(); ++i) {
                int from = result.path[i];
                int to   = result.path[i + 1];

                for (const auto& edge : city.getNeighbors(from)) {
                    if (edge.to == to) {
                        json edgeData;
                        edgeData["from"]         = from;
                        edgeData["to"]           = to;
                        edgeData["quality"]      = edge.quality;
                        edgeData["blocked"]      = edge.blocked;
                        edgeData["trafficLevel"] = edge.trafficLevel;
                        edgesInfo.push_back(edgeData);
                        break;
                    }
                }
            }
        }
        response["edges"] = edgesInfo;

        response["mainWarehouse"]      = manager.getMainWarehouse();
        response["deliveryWarehouses"] = manager.getDeliveryWarehouses();
    }


    else if (action == "delivery") {

        auto smartFunc = [](const Edge& e) -> double {
            if (e.blocked) return std::numeric_limits<double>::infinity();
            double time = e.time * e.trafficLevel;
            return time;
        };


        int mainWH = manager.getMainWarehouse();
        auto deliveries = manager.getDeliveryWarehouses();

        if (deliveries.empty() || mainWH < 0) {
            response["fullPath"] = json::array();
            response["segments"] = json::array();
            response["totalTime"] = 0.0;
            response["successCount"] = 0;
            response["totalWarehouses"] = deliveries.size();
            response["mainWarehouse"] = mainWH;
            response["deliveryWarehouses"] = deliveries;
        } else {
            std::vector<int> unvisited = deliveries;
            std::vector<int> fullPath;
            json segments = json::array();

            bool firstSegment = true;
            double totalTime = 0.0;
            int successCount = 0;
            int current = mainWH;

            while (!unvisited.empty()) {
                int bestTarget = -1;
                PathResult bestResult;
                double bestWeight = std::numeric_limits<double>::infinity();

                for (int target : unvisited) {
                    PathResult res = Algorithms::dijkstra(city, current, target, smartFunc);
                    if (res.found && res.totalWeight < bestWeight) {
                        bestWeight = res.totalWeight;
                        bestResult = res;
                        bestTarget = target;
                    }
                }

                if (bestTarget == -1) {
                    break;
                }

                if (firstSegment) {
                    fullPath = bestResult.path;
                    firstSegment = false;
                } else {
                    fullPath.insert(fullPath.end(),
                                    bestResult.path.begin() + 1,
                                    bestResult.path.end());
                }

                json seg;
                seg["from"] = current;
                seg["to"] = bestTarget;
                seg["path"] = bestResult.path;
                seg["time"] = bestResult.totalWeight;
                segments.push_back(seg);

                totalTime += bestResult.totalWeight;
                successCount++;

                unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), bestTarget),
                                unvisited.end());

                current = bestTarget;
            }

            response["fullPath"] = fullPath;
            response["segments"] = segments;
            response["totalTime"] = totalTime;
            response["successCount"] = successCount;
            response["totalWarehouses"] = deliveries.size();
            response["mainWarehouse"] = mainWH;
            response["deliveryWarehouses"] = deliveries;
        }
    }

    else if (action == "edit_edge") {
        int from = std::stoi(params.count("from") ? params["from"] : "-1");
        int to   = std::stoi(params.count("to")   ? params["to"]   : "-1");
        std::string mode = params.count("mode") ? params["mode"] : "green";

        if (from < 0 || to < 0 || from >= city.getVertexCount() || to >= city.getVertexCount()) {
            response["status"] = "error";
            response["message"] = "Invalid vertex indices";
        } else {

            auto& n1 = city.getNeighbors(from);
            auto& n2 = city.getNeighbors(to);

            auto applyMode = [&](auto& neighbors, int target) {
                for (auto& edge : neighbors) {
                    if (edge.to == target) {
                        if (mode == "green") {
                            edge.blocked = false;
                            edge.trafficLevel = 1.0;
                        } else if (mode == "yellow") {
                            edge.blocked = false;
                            edge.trafficLevel = 1.5;
                        } else if (mode == "orange") {
                            edge.blocked = false;
                            edge.trafficLevel = 2.5;
                        } else if (mode == "block") {
                            edge.blocked = true;
                        } else if (mode == "unblock") {
                            edge.blocked = false;
                        }
                        break;
                    }
                }
            };

            applyMode(n1, to);
            applyMode(n2, from);

            // Сохраняем изменения в graph.json
            city.exportToJSON("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/graph.json");

            response["status"] = "success";
            response["from"] = from;
            response["to"] = to;
            response["mode"] = mode;
        }
    }


    else if (action == "regenerate") {
        int numDelivery = std::stoi(params.count("num") ? params["num"] : "5");
        manager.generateWarehouses(city, numDelivery);

        saveWarehouses(manager);

        response["status"] = "success";
        response["message"] = "Warehouses regenerated";
        response["mainWarehouse"] = manager.getMainWarehouse();
        response["deliveryWarehouses"] = manager.getDeliveryWarehouses();
    }

    else if (action == "set_main") {
        int vertexId = std::stoi(params.count("vertex") ? params["vertex"] : "-1");
        if (vertexId >= 0 && vertexId < city.getVertexCount()) {
            manager.setMainWarehouse(vertexId);
            saveWarehouses(manager);

            response["status"] = "success";
            response["message"] = "Main warehouse set";
            response["mainWarehouse"] = manager.getMainWarehouse();
            response["deliveryWarehouses"] = manager.getDeliveryWarehouses();
        } else {
            response["status"] = "error";
            response["message"] = "Invalid vertex ID";
        }
    }

    else if (action == "add_delivery") {
        int vertexId = std::stoi(params.count("vertex") ? params["vertex"] : "-1");
        if (vertexId >= 0 && vertexId < city.getVertexCount()) {
            manager.addDeliveryWarehouse(vertexId);
            saveWarehouses(manager);

            response["status"] = "success";
            response["message"] = "Delivery warehouse added";
            response["mainWarehouse"] = manager.getMainWarehouse();
            response["deliveryWarehouses"] = manager.getDeliveryWarehouses();
        } else {
            response["status"] = "error";
            response["message"] = "Invalid vertex ID";
        }
    }

    else if (action == "remove_delivery") {
        int vertexId = std::stoi(params.count("vertex") ? params["vertex"] : "-1");
        manager.removeDeliveryWarehouse(vertexId);
        saveWarehouses(manager);

        response["status"] = "success";
        response["message"] = "Delivery warehouse removed";
        response["mainWarehouse"] = manager.getMainWarehouse();
        response["deliveryWarehouses"] = manager.getDeliveryWarehouses();
    }

    else {
        response["error"] = "Unknown action";
        response["allowed"] = {"generate", "find", "delivery", "regenerate", "set_main", "add_delivery", "remove_delivery"};
                                                                        // чтобы не было null в выводе
    }

    std::cout.rdbuf(oldCout);

    std::ofstream responseFile("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/response.json");
    if (responseFile.is_open()) {
        responseFile << response.dump(2);
        responseFile.close();
    }

    if (action == "delivery" || action == "find") {
        std::ofstream deliveryFile("/Users/maxshushalov/CPP_projects/Tiger/Laba_3/web/delivery.json");
        if (deliveryFile.is_open()) {
            deliveryFile << response.dump(2);
            deliveryFile.close();
        }
    }

    return 0;

}
