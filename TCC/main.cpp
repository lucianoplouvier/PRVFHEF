#include <iostream>
#include <fstream>
#include <string>
#include "heterogeneousTransports.h"
using namespace std;

std::vector<std::string> strSplit(const std::string& text, char separator) {
    std::vector<std::string> result;
    std::size_t start = 0, end = 0;

    while ((end = text.find(separator, start)) != std::string::npos)
    {
        result.push_back(text.substr(start, end - start));
        start = end + 1;
    }

    result.push_back(text.substr(start));
    return result;
}

bool readProblem(std::string input, std::vector<float>& demands, std::vector<Vehicle>& vehicleTypes, std::vector<ClientAdjacency>& adjacencies, std::vector<float>& depotTravelCosts) {
    /*
    * Formato do arquivo
    * -> numero de clientes
    * -> lista de demandas de cada cliente
    * -> numero de veiculos
    * ->(repete para cada veiculo) capacidade veiculo, tipo veiculo, custo veiculo, custo por km andado
    * -> lista de adjacencias ao depot
    * -> numero de adjacencias
    * ->(repete para cada adjacencia) idCliente1 idCliente2 custoAdjacencia. Começa no 1 pois 0 = depot
    */

    string line;

    int clients = 0;

    int vehicles = 0;

    int numberAdjacencies = 0;

    ifstream myfile;
    myfile.open(input.c_str());

    if (myfile.is_open()) {
        if (getline(myfile, line)) {
            clients = stoi(line);
            demands.reserve(clients);
        }
        else {
            return false;
        }
        if (getline(myfile, line)) {
            std::vector<std::string> lineSplit = strSplit(line, ' ');
            for (int i = 0; i < lineSplit.size(); i++) {
                demands.push_back(atoi(lineSplit.at(i).c_str()));
            }
        }
        else {
            return false;
        }
        if (getline(myfile, line)) {
            vehicles = stoi(line);
            vehicleTypes.reserve(vehicles);
            for (int i = 0; i < vehicles; i++) {
                if (getline(myfile, line)) {
                    std::vector<std::string> lineSplit = strSplit(line, ' ');
                    Vehicle newVehicleType;
                    newVehicleType.id = i;
                    newVehicleType.capacity = atoi(lineSplit[0].c_str());
                    newVehicleType.cost = atoi(lineSplit[2].c_str());
                    newVehicleType.travelCost = atoi(lineSplit[3].c_str());
                    vehicleTypes.push_back(newVehicleType);
                }
                else {
                    return false;
                }
            }
        }
        if (getline(myfile, line)) {
            std::vector<std::string> lineSplit = strSplit(line, ' ');
            for (int i = 0; i < lineSplit.size(); i++) {
                depotTravelCosts.push_back(atoi(lineSplit.at(i).c_str()));
            }
        }
        else {
            return false;
        }
        if (getline(myfile, line)) {
            numberAdjacencies = stoi(line);
            adjacencies.reserve(numberAdjacencies);
            for (int i = 0; i < numberAdjacencies; i++) {
                if (getline(myfile, line)) {
                    std::vector<std::string> lineSplit = strSplit(line, ' ');
                    ClientAdjacency adjacency;
                    adjacency.clientFromId = atoi(lineSplit[0].c_str());
                    adjacency.clientToId = atoi(lineSplit[1].c_str());
                    adjacency.travelCost = atoi(lineSplit[2].c_str());
                    adjacencies.push_back(adjacency);
                }
                else {
                    return false;
                }
            }
        }
    }

    return true;
}

bool readProblemText(std::string input, std::vector<float>& demands, std::vector<Vehicle>& vehicleTypes, std::vector<ClientAdjacency>& adjacencies, std::vector<float>& depotTravelCosts) {

    string line;

    int clients = 0;

    int vehicles = 0;

    int depotX, depotY;

    std::vector<std::pair<int, int>> clientCoords;

    std::vector<int> availableVels;

    ifstream myfile;
    myfile.open(input.c_str());

    if (myfile.is_open()) {
        if (getline(myfile, line)) {
            clients = stoi(line);
            demands.reserve(clients);
        }
        else {
            return false;
        }
        if (getline(myfile, line)) {
            std::vector<std::string> lineSplit = strSplit(line, ' ');
            depotX = (atoi(lineSplit.at(1).c_str()));
            depotY = (atoi(lineSplit.at(2).c_str()));
        }
        for (int i = 0; i < clients; i++) {
            if (getline(myfile, line)) {
                std::vector<std::string> lineSplit = strSplit(line, ' ');
                int x = atoi(lineSplit.at(1).c_str());
                int y = atoi(lineSplit.at(2).c_str());
                std::pair<int, int> coord(x, y);
                clientCoords.push_back(coord);
                demands.push_back(atoi(lineSplit.at(3).c_str()));
            }
            else {
                return false;
            }
        }
        getline(myfile, line); // pulac comentario
        if (getline(myfile, line)) {
            vehicles = stoi(line);
            vehicleTypes.reserve(vehicles);
            for (int i = 0; i < vehicles; i++) {
                if (getline(myfile, line)) {
                    std::vector<std::string> lineSplit = strSplit(line, ' ');
                    Vehicle newVehicleType;
                    newVehicleType.id = i;
                    newVehicleType.capacity = atoi(lineSplit[0].c_str());
                    newVehicleType.cost = atoi(lineSplit[1].c_str());
                    if (lineSplit.size() > 2) {
                        newVehicleType.travelCost = atoi(lineSplit[3].c_str());
                    }
                    else {
                        newVehicleType.travelCost = 1;
                    }
                    if (lineSplit.size() > 3) {
                        availableVels.push_back(atoi(lineSplit[3].c_str()));
                    }
                    vehicleTypes.push_back(newVehicleType);
                }
                else {
                    return false;
                }
            }
        }
        else {
            return false;
        }
    }

    for (int i = 0; i < clientCoords.size(); i++) {
        auto cCoord = clientCoords[i];
        for (int j = 0; j < clientCoords.size(); j++) {
            if (i != j) {
                auto otherCoord = clientCoords[j];
                ClientAdjacency adj;
                adj.clientFromId = i;
                adj.clientToId = j;
                adj.travelCost = std::abs(std::abs(cCoord.first) - std::abs(otherCoord.first)) + std::abs(std::abs(cCoord.second) - std::abs(otherCoord.second));
                adjacencies.push_back(adj);
            }
        }
        int depotCost = std::abs(std::abs(cCoord.first) - std::abs(depotX)) + std::abs(std::abs(cCoord.second) - std::abs(depotY));
        depotTravelCosts.push_back(depotCost);
    }

    return true;
}

int main()
{
    std::vector<float> demands;
    std::vector<ClientAdjacency> adjacencies;
    std::vector<Vehicle> vehicleTypes;
    std::vector<float> depotTravelCosts;

    //std::string filepath = "C:/Users/frien/Documents/testeTCC.txt";
    
    //readProblem(filepath, demands, vehicleTypes, adjacencies, depotTravelCosts);

    std::string filepath = "C:/Users/frien/Documents/c20_3.txt";

    //std::string filepath = "C:/Users/frien/Documents/c20_4mix.txt";

    //std::string filepath = "C:/Users/frien/Documents/c20_5mix.txt";

    //std::string filepath = "C:/Users/frien/Documents/c20_6mix.txt";

    readProblemText(filepath, demands, vehicleTypes, adjacencies, depotTravelCosts);

    PRVFHEF transport(demands, adjacencies, vehicleTypes, depotTravelCosts);
}