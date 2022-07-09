#include "route.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <stdio.h> 

using namespace std;

RouteCreator::RouteCreator() {

}

RouteCreator::~RouteCreator() {

}

Route RouteCreator::createRoute(Vehicle v) {
	Route r(m_lastId, v);
	m_lastId++;
	if (m_lastId == std::numeric_limits<int>::max()) {
		cout << "Limite de ids atingido. Retornando a 0.\n";
		m_lastId = 0;
	}
	return r;
}

double RouteDefs::calculateTravelCost(const std::vector<Client>& clients, const AdjacencyCosts& adjacencyCosts) {
	double total = 0;
	if (clients.size() > 0) {
		int firstClientId = clients[0].id;
		int lastClientId = clients[clients.size() - 1].id;
		double depotTravelStart = adjacencyCosts.depotTravel[firstClientId];
		double depotTravelEnd = adjacencyCosts.depotTravel[lastClientId]; // Voltar ao depósito.
		total += depotTravelStart; // Sair do deposito.
		total += depotTravelEnd;
		for (int iClient = 0; iClient < clients.size() - 1; iClient++) {
			int currClientId = clients[iClient].id;
			int nextClientId = clients[iClient + 1].id;
			double cost = adjacencyCosts.costs[currClientId][nextClientId];
			total += cost;
		}
	}
	return total;
}

double RouteDefs::evaluate(const std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts) {
	double total = 0;
	for (const Route& route : solution) {
		if (!route.clientsList.empty()) {
			total += route.vehicle.cost;
			double routeCost = RouteDefs::evaluateRoute(route, adjacencyCosts);
			total += routeCost;
		}
	}
	return total;
}

double RouteDefs::evaluateRoute(const Route& route, const AdjacencyCosts& adjacencyCosts) {
	return calculateTravelCost(route.clientsList, adjacencyCosts) * route.vehicle.travelCost;
}

void RouteDefs::printRoute(const Route& route) {
	for (const Client& c : route.clientsList) {
		cout << "(" << c.id << "," << c.demand << "),";
	}
	cout << "\n";
}

void RouteDefs::swapClients(Route& r1, int r1ClientIndex, Route& r2, int r2clientIndex) {
	// O novo cliente é o cliente da outra rota.
	Client newR1Client(r2.clientsList[r2clientIndex]);
	Client newR2Client(r1.clientsList[r1ClientIndex]);
	r1.clientsList[r1ClientIndex] = newR1Client;
	r2.clientsList[r2clientIndex] = newR2Client;
}

void RouteDefs::swapClients(Route& r1, int r1ClientId1, int r1ClientId2, Route& r2, int r2ClientId) {
	printRoute(r1);
	printRoute(r2);

	int r1_1Index = r1.findClient(r1ClientId1);
	int r1_2Index = r1.findClient(r1ClientId2);

	int r2Index = r1.findClient(r2ClientId);

	Client clientR1_1(r1.clientsList[r1_1Index]);
	Client clientR1_2(r1.clientsList[r1_2Index]);

	Client clientR2(r1.clientsList[r2Index]);

	r1.removeClient(clientR1_2);
	r1.clientsList[r1_1Index] = clientR2;

	r2.clientsList[r1_1Index] = clientR1_1;
	r2.insertClient(clientR1_2, r1_2Index);

	printRoute(r1);
	printRoute(r2);
}

std::pair<double, int> RouteDefs::findBestInsertion(const Route& route, const std::vector<Client>& clientsList, const AdjacencyCosts& adjacencyCosts, int forbiddenIndex) {
	int firstClientId = clientsList.front().id;
	double firstClientdemand = clientsList.front().demand;
	int lastClientId = clientsList.back().id;
	double lastClientDemand = clientsList.back().demand;
	std::pair<double, int> result;
	double cost = 0;
	int pos = 0;
	// Se colocar na primeira posição.
	cost = adjacencyCosts.depotTravel[firstClientId];
	pos = 0;

	int clients = route.clientsList.size();
	if (clients > 0) {
		cost += adjacencyCosts.getAdjacencyCosts(lastClientId, route.clientsList[0].id);
	}

	if (clients == 0) { // Inserir em rota  vazia.
		result.first = cost + adjacencyCosts.depotTravel[lastClientId];
		result.second = pos;
		return result;
	}
	else {
		for (int i = 0; i <= clients; i++) {
			int currClientId = -1;
			if (i != forbiddenIndex) {
				if (i != clients) {
					currClientId = route.clientsList[i].id;
				}
				double currCost = 0;
				if (i == 0) { // Colocar um cliente no inicio
					currCost = adjacencyCosts.depotTravel[firstClientId] + adjacencyCosts.getAdjacencyCosts(lastClientId, currClientId) - adjacencyCosts.depotTravel[currClientId];
				}
				else if (i == clients) { // Colocar um cliente no fim
					currCost = adjacencyCosts.getAdjacencyCosts(firstClientId, route.clientsList[i - 1].id) + adjacencyCosts.depotTravel[lastClientId] - adjacencyCosts.depotTravel[route.clientsList[i - 1].id];
				}
				else {
					int prevId = route.clientsList[i - 1].id;
					currCost = adjacencyCosts.getAdjacencyCosts(prevId, firstClientId) + adjacencyCosts.getAdjacencyCosts(lastClientId, currClientId) - adjacencyCosts.getAdjacencyCosts(prevId, currClientId); // Colocar um cliente entre dois é somar as duas viagems que isso envolve.
				}
				if (currCost < cost) {
					cost = currCost;
					pos = i;
				}
			}
		}
		result.first = cost * route.vehicle.travelCost + route.vehicle.cost; // Levar em conta o custo de viagem + custo do veículo.
		result.second = pos;
		return result;
	}
}

bool RouteDefs::isSolutionValid(const std::vector<Route>& solution, std::vector<Client> completeClientList, std::vector<int> availableVels) {
	int availableVelsSize = availableVels.size();
	bool hasLimitedVels = availableVelsSize > 0;
	std::vector<int> demandsApplied;
	for (int i = 0; i < completeClientList.size(); i++) {
		demandsApplied.push_back(completeClientList[i].demand);
	}
	for (int i = 0; i < solution.size(); i++) {
		double totalDemand = solution[i].getTotalDemand();
		if (totalDemand > solution[i].vehicle.capacity) {
			//cout << "ERRO CAPACIDADE VEICULO\n";
			return false; // Se entrar aqui é que estourou a capacidade
		}
		if (hasLimitedVels) {
			int velLeft = availableVels[solution[i].vehicle.id]--;
			if (velLeft < 0) {
				//cout << "ERRO QNT VEICULOS\n";
				return false; // Se entrar aqui é que tem veículo demais em uso
			}
		}
		std::vector<int> idsVisited; // Ver se não tem o mesmo cliente duas vezes.
		for (int j = 0; j < solution[i].clientsList.size(); j++) {
			int id = solution[i].clientsList[j].id;
			/*
			for (int idVisited : idsVisited) {
				if (idVisited == id) {
					//return false;
				}
			}
			*/
			idsVisited.push_back(id);
			demandsApplied[id] -= solution[i].clientsList[j].demand;
		}
	}

	for (int i = 0; i < completeClientList.size(); i++) {
		if (demandsApplied[i] > 0 || demandsApplied[i] < 0) {
			//cout << "ERRO DEMANDA\n";
			return false; // Se entrar aqui é que sumiu demanda
		}
	}

	return true;
}

Vehicle RouteDefs::getBiggestVehicle(const std::vector<Vehicle>& vehiclesList) {
	Vehicle biggestVehicle = vehiclesList[0];
	for (auto iVels = 1; iVels < vehiclesList.size(); iVels++) {
		if (vehiclesList[iVels].capacity > biggestVehicle.capacity) {
			biggestVehicle = vehiclesList[iVels];
		}
	}
	return biggestVehicle;
}

Vehicle RouteDefs::getSmallestVehicle(const std::vector<Vehicle>& vehiclesList) {
	Vehicle smallestVehicle = vehiclesList[0];
	for (auto iVels = 1; iVels < vehiclesList.size(); iVels++) {
		if (vehiclesList[iVels].capacity < smallestVehicle.capacity) {
			smallestVehicle = vehiclesList[iVels];
		}
	}
	return smallestVehicle;
}

bool RouteDefs::fitsInNonBiggestVehicle(double demand, const std::vector<Vehicle>& vehiclesList) {
	Vehicle biggest = RouteDefs::getBiggestVehicle(vehiclesList);
	for (Vehicle v : vehiclesList) {
		if (v.id != biggest.id && demand <= v.capacity) {
			return true;
		}
	}
	return false;
}

int RouteDefs::removeClientFromSolution(std::vector<Route>& solution, int clientId, int& routeIndexSingleRemoval) {
	int count = 0;
	for (int i = 0; i < solution.size(); i++) {
		Route& r = solution[i];
		bool success = r.removeClient(clientId);
		if (success) {
			count++;
			routeIndexSingleRemoval = i;
		}
	}
	if (count > 1) {
		routeIndexSingleRemoval = -1;
	}
	return count;
}

Client RouteDefs::getOriginalClient(int clientId, const std::vector<Client>& clientList) {
	for (Client c : clientList) {
		if (c.id == clientId) return c;
	}
	cout << "ERROR. Cliente " << clientId << " não encontrado.";
	exit(1);
}

std::vector<int> RouteDefs::calculateAvailableVels(const std::vector<Route>& solution, std::vector<int> availableVels) {
	for (const Route& r : solution) {
		availableVels[r.vehicle.id] = availableVels[r.vehicle.id] - 1;
	}
	return availableVels;
}