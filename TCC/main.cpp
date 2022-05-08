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

bool readProblem(std::string input, std::vector<int>& demands, std::vector<Vehicle>& vehicleTypes, std::vector<ClientAdjacency>& adjacencies) {
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
                    newVehicleType.cost = atoi(lineSplit[0].c_str());
                    newVehicleType.type = atoi(lineSplit[1].c_str());
                    newVehicleType.capacity = atoi(lineSplit[2].c_str());
                    vehicleTypes.push_back(newVehicleType);
                }
                else {
                    return false;
                }
            }
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

int main()
{
    std::vector<int> demands;
    std::vector<ClientAdjacency> adjacencies;
    std::vector<Vehicle> vehicleTypes;

    std::string filepath = "C:/Users/frien/Documents/testeTCC.txt";
    
    readProblem(filepath, demands, vehicleTypes, adjacencies);

    PRVFHEF transport(demands, adjacencies, vehicleTypes);
}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
