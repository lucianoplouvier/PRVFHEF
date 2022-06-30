#pragma once
#include <vector>
#include <list>
#include "structures.h"
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>

using namespace std;

enum class INTERROUTETYPES {
	SHIFT1_0,
	SHIFT2_0,
	SWAP1_1,
	SWAP2_1,
	CROSS,
	SWAP1_1S,
	SWAP2_1S,
	KSPLIT
};

enum class INTRAROUTETYPES {
	EXCHANGE, // Swap
	REINSERTION, // Shift
	TWO_OPT,
	OR_OPT2,
	OR_OPT3,
	REVERSE
};

struct Route {
	int id;
	std::vector<Client> clientsList;
	Vehicle vehicle;
	Route(const Route& other) {
		this->id = other.id;
		this->vehicle = other.vehicle;
		int clients = other.clientsList.size();
		this->clientsList.reserve(clients);
		for (int i = 0; i < clients; i++) {
			this->clientsList.push_back(Client(other.clientsList[i]));
		}
	}
	Route(int id, Vehicle v) {
		this->id = id;
		this->vehicle = v;
	}

	bool canAddClient(double demand) const {
		double totalDemand = getTotalDemand();
		double left = vehicle.capacity - totalDemand;
		return demand <= left;
	}

	bool addClient(int clientIndex, double demand) {
		bool canAdd = this->canAddClient(demand);
		if (canAdd) {
			Client c;
			c.id = clientIndex;
			c.demand = demand;
			clientsList.push_back(c);
		}
		return canAdd;
	}

	bool addClient(Client c) {
		bool canAdd = this->canAddClient(c.demand);
		if (canAdd) {
			clientsList.push_back(c);
		}
		return canAdd;
	}

	// Junta a demanda do cliente caso o cliente já exista na rota.
	bool addClientOrMerge(int clientId, double demand) {
		bool canAdd = this->canAddClient(demand);
		if (canAdd) {
			bool done = false;
			for (int i = 0; i < clientsList.size(); i++) {
				if (clientsList[i].id == clientId) {
					clientsList[i].demand += demand;
					done = true;
					break;
				}
			}
			if (!done) {
				addClient(clientId, demand);
			}
		}
		return canAdd;
	}

	bool insertClient(const Client& client, int position) {
		bool canAdd = this->canAddClient(client.demand);
		if (canAdd) {
			Client c(client);
			std::vector<Client>::iterator it = this->clientsList.begin();
			while (position > 0 && it != this->clientsList.end()) {
				it++;
				position--;
			}
			if (it == this->clientsList.end()) {
				clientsList.push_back(c);
			}
			else {
				clientsList.insert(it, c);
			}
		}
		return canAdd;
	}

	Client takeClient(int position) {
		if (position >= this->clientsList.size()) {
			cout << "Erro no index de takeClient";
			exit(1);
		}
		auto iterator = clientsList.begin();
		int i = 0;
		while (iterator != clientsList.end() && i < position) {
			i++;
			iterator++;
		}
		Client c = clientsList[i];
		clientsList.erase(iterator);
		return c;
	}

	bool removeClient(const Client& client) { // Retorna true se obteve sucesso, false caso contário.
		if (clientsList.size() > 0) {
			auto iterator = clientsList.begin();
			while (iterator != clientsList.end()) {
				if (iterator->id == client.id && iterator->demand == client.demand) {
					break;
				}
				iterator++;
			}
			if (iterator != clientsList.end()) {
				clientsList.erase(iterator);
				return true;
			}
		}
		return false;
	}

	bool removeClient(int clientId) { // Retorna true se obteve sucesso, false caso contário.
		bool found = false;
		if (clientsList.size() > 0) {
			auto iterator = clientsList.begin();
			while (iterator != clientsList.end()) {
				if (iterator->id == clientId) {
					clientsList.erase(iterator);
					iterator = clientsList.begin(); // TODO Ineficiente
					found = true;
				}
				else {
					iterator++;
				}
			}
		}
		return found;
	}

	bool removeClient(Client& c) {
		for (int i = 0; i < clientsList.size(); i++) {
			Client& curr = clientsList[i];
			if(c == curr){
				std::vector<Client>::iterator it = clientsList.begin();
				std::advance(it, i);
				clientsList.erase(it);
				return true;
			}
		}
		cout << "ERROR. Remove Client falhou.\n";
		return false;
	}

	int findClient(int clientId) const {
		int index = 0;
		auto iterator = this->clientsList.begin();
		while (iterator != clientsList.end()) {
			if (iterator->id == clientId) {
				return index;
			}
			iterator++;
			index++;
		}
		return -1;
	}

	double getTotalDemand() const {
		double totalDemand = 0;
		for (int i = 0; i < (int)clientsList.size(); i++) {
			totalDemand += clientsList[i].demand;
		}
		return totalDemand;
	}

	inline bool operator==(const Route& other) {
		return other.id == this->id;
	}

	double getDemand(int indexStart, int indexEnd) const {
		double result = 0;
		for (int i = indexStart; i <= indexEnd; i++) {
			result += clientsList[i].demand;
		}
		return result;
	}

	double minDelivery() const {
		if (clientsList.size() == 0) return 0;
		double result = clientsList[0].demand;
		for (int i = 1; i < clientsList.size(); i++) {
			double d = clientsList[i].demand;
			if (d < result) result = d;
		}
		return result;
	}

	double minDeliveryAdj() const {
		if (clientsList.size() == 0) return 0;
		double result = clientsList[0].demand;
		for (int i = 1; i < clientsList.size(); i++) {
			double d = clientsList[i].demand;
			if (i + 1 < clientsList.size()) {
				d += clientsList[i + 1].demand;
			}
			if (d < result) result = d;
		}
		return result;
	}

	double maxDelivery() const {
		if (clientsList.size() == 0) return 0;
		double result = clientsList[0].demand;
		for (int i = 1; i < clientsList.size(); i++) {
			double d = clientsList[i].demand;
			if (d > result) result = d;
		}
		return result;
	}
};

struct AdjacencyCosts {
	std::vector<std::vector<double>> costs;
	std::vector<double> depotTravel;
	double getAdjacencyCosts(int client1Index, int client2Index) const { return costs[client1Index][client2Index]; }
};

namespace RouteDefs {

	double calculateTravelCost(const std::vector<Client>& clients, const AdjacencyCosts& adjacencyCosts);

	double evaluate(const std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts);

	double evaluateRoute(const Route& route, const AdjacencyCosts& adjacencyCosts);

	void printRoute(const Route& route);

	void swapClients(Route& r1, int r1clientIndex, Route& r2, int r2clientIndex);

	void swapClients(Route& r1, int r1ClientId1, int r1ClientId2, Route& r2, int r2ClientId);

	/*
	* @brief Encontra a melhor inserção do cliente informado na rota.
	* @param route - Rota.
	* @param clientsList - lista de clientes a se inserir.
	* @param adjacencyCosts - Lista de adjacencias.
	* @param indice proibido, por padrão nenhum (-1).
	* @return Par, onde o primeiro valor = custo, e o segundo valor é o indice.
	*/
	std::pair<double, int> findBestInsertion(const Route& route, const std::vector<Client>& clientsList, const AdjacencyCosts& adjacencyCosts, int forbiddenIndex = -1);

	/*
	* @brief Encontra a melhor inserção do cliente informado na rota.
	* @param route - Rota.
	* @param client - Cliente.
	* @param adjacencyCosts - Lista de adjacencias.
	* @return Par, onde o primeiro valor = custo, e o segundo valor é o indice.
	*/
	//std::pair<double, int> findBestInsertion(const Route& route, const Client& client, const AdjacencyCosts& adjacencyCosts);

	/*
	* @brief Encontra a melhor inserção do cliente informado na rota.
	* @param route - Rota.
	* @param client - Cliente 1.
	* @param next - Cliente adjacente.
	* @param adjacencyCosts - Lista de adjacencias.
	* @return Par, onde o primeiro valor = custo, e o segundo valor é o indice.
	*/
	//std::pair<double, int> findBestInsertion(Route& route, Client& client, Client& next, const AdjacencyCosts& adjacencyCosts);

	bool isSolutionValid(const std::vector<Route>& solution, std::vector<Client> completeClientList, std::vector<int> availableVels);

	/*
	* @brief Recupera o menor veículo disponível.
	* @return Maior veículo na lista de veículos.
	*/
	Vehicle getBiggestVehicle(const std::vector<Vehicle>& vehiclesList);

	/*
	* @brief Recupera o menor veículo disponível.
	* @return Menor veículo na lista de veículos.
	*/
	Vehicle getSmallestVehicle(const std::vector<Vehicle>& vehiclesList);

	/*
	* @brief Remove um cliente da solução.
	* @param solution - Solução.
	* @param clientId - Id do cliente a se remover.
	* @param routeIdSingleRemoval - Retorna a posição da rota na solução que o cliente foi removido, ou -1 se for removido em mais de uma rota.
	* @return quantidade de vezes que ele foi removido.
	*/
	int removeClientFromSolution(std::vector<Route>& solution, int clientId, int& routeIndexSingleRemoval);

	Client getOriginalClient(int clientId, const std::vector<Client>& clientList);

	bool fitsInNonBiggestVehicle(double demand, const std::vector<Vehicle>& vehiclesList);

	std::vector<int> calculateAvailableVels(const std::vector<Route>& solution, std::vector<int> availableVels);
}

class RouteCreator {

public:

	RouteCreator();

	~RouteCreator();

	Route createRoute(Vehicle v);

	void reset() {
		m_lastId = 0;
	}

private:

	int m_lastId = 0;


};