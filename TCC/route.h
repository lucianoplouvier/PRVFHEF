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
	SWAP1_1S,
	SWAP2_1S,
	ROUTEADDITION,
	KSPLIT
};

enum class INTRAROUTETYPES {
	EXCHANGE, // Swap
	REINSERTION, // Shift
	TWO_OPT,
	OR_OPT2,
	OR_OPT3
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

	bool canAddClient(int demand) const {
		float totalDemand = getTotalDemand();
		float left = vehicle.capacity - totalDemand;
		return demand <= left;
	}

	bool addClient(int clientIndex, int demand) {
		bool canAdd = this->canAddClient(demand);
		if (canAdd) {
			Client c;
			c.id = clientIndex;
			c.demand = demand;
			clientsList.push_back(c);
		}
		return canAdd;
	}

	// Junta a demanda do cliente caso o cliente já exista na rota.
	bool addClientOrMerge(int clientId, int demand) {
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
	/*
	bool insertClient(int clientIndex, int demand, int position) {
		bool canAdd = this->canAddClient(demand);
		if (canAdd) {
			Client c;
			c.id = clientIndex;
			c.demand = demand;
			std::vector<Client>::iterator it = this->clientsList.begin();
			while (position > 0) {
				it++;
				position--;
			}
			clientsList.insert(it, c);
		}
		return canAdd;
	}
	*/
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

	void removeClient(const Client& client) {
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
			}
		}
	}
	/*
	void removeClient(int clientId, int clientDemand) {
		if (clientsList.size() > 0) {
			auto iterator = clientsList.begin();
			while (iterator != clientsList.end()) {
				if (iterator->id == clientId && iterator->demand == clientDemand) {
					break;
				}
				iterator++;
			}
			if (iterator != clientsList.end()) {
				clientsList.erase(iterator);
			}
		}
	}
	*/
	int findClient(int clientId) {
		int index = 0;
		auto iterator = this->clientsList.begin();
		while (iterator != clientsList.end() && iterator->id != clientId) {
			iterator++;
			index++;
		}
		if (iterator < clientsList.end()) {
			return index;
		}
		else {
			return -1;
		}
	}

	int getTotalDemand() const {
		int totalDemand = 0;
		for (int i = 0; i < (int)clientsList.size(); i++) {
			totalDemand += clientsList[i].demand;
		}
		return totalDemand;
	}

	inline bool operator==(const Route& other) {
		return other.id == this->id;
	}
};

struct AdjacencyCosts {
	std::vector<std::vector<float>> costs;
	std::vector<float> depotTravel;
	float getAdjacencyCosts(int client1Index, int client2Index) const { return costs[client1Index][client2Index]; }
};

namespace RouteDefs {

	float calculateTravelCost(const std::vector<Client>& clients, const AdjacencyCosts& adjacencyCosts);

	float evaluate(std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts);

	float evaluateRoute(const Route& route, const AdjacencyCosts& adjacencyCosts);

	void printRoute(const Route& route);

	void swapClients(Route& r1, int r1clientIndex, Route& r2, int r2clientIndex);

	void swapClients(Route& r1, int r1ClientId1, int r1ClientId2, Route& r2, int r2ClientId);

	/*
	* @brief Encontra a melhor inserção do cliente informado na rota.
	* @param route - Rota.
	* @param clientsList - cadeia de clientes adjacentes.
	* @param adjacencyCosts - Lista de adjacencias.
	* @return Par, onde o primeiro valor = custo, e o segundo valor é o indice.
	*/
	std::pair<float, int> findBestInsertion(const Route& route, const std::vector<Client>& clientsList, const AdjacencyCosts& adjacencyCosts);

	/*
	* @brief Encontra a melhor inserção do cliente informado na rota.
	* @param route - Rota.
	* @param client - Cliente.
	* @param adjacencyCosts - Lista de adjacencias.
	* @return Par, onde o primeiro valor = custo, e o segundo valor é o indice.
	*/
	//std::pair<float, int> findBestInsertion(const Route& route, const Client& client, const AdjacencyCosts& adjacencyCosts);

	/*
	* @brief Encontra a melhor inserção do cliente informado na rota.
	* @param route - Rota.
	* @param client - Cliente 1.
	* @param next - Cliente adjacente.
	* @param adjacencyCosts - Lista de adjacencias.
	* @return Par, onde o primeiro valor = custo, e o segundo valor é o indice.
	*/
	//std::pair<float, int> findBestInsertion(Route& route, Client& client, Client& next, const AdjacencyCosts& adjacencyCosts);

	std::vector<Route> copy(const std::vector<Route>& other);

	bool isSolutionValid(const std::vector<Route>& solution, std::vector<Client> completeClientList);

	/*
	* @brief Recupera o maior veículo disponível.
	* @return Maior veículo na lista de veículos.
	*/
	Vehicle getBiggestVehicle(const std::vector<Vehicle>& vehiclesList);
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