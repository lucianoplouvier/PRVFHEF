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

float RouteDefs::calculateTravelCost(const std::vector<Client>& clients, const AdjacencyCosts& adjacencyCosts) {
	float total = 0;
	if (clients.size() > 0) {
		int firstClientIndex = clients[0].id;
		int lastClientIndex = clients[clients.size() - 1].id;
		total += adjacencyCosts.depotTravel[firstClientIndex]; // Sair do deposito.
		total += adjacencyCosts.depotTravel[lastClientIndex]; // Voltar do depósito.
		for (int iClient = 0; iClient < clients.size() - 1; iClient++) {
			total += adjacencyCosts.costs[iClient][iClient + 1];
		}
	}
	return total;
}

float RouteDefs::evaluate(std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts) {
	float total = 0;
	for (int iRoute = 0; iRoute < solution.size(); iRoute++) {
		Route& route = solution[iRoute];
		total += route.vehicle.cost;
		float totalTravel = RouteDefs::calculateTravelCost(route.clientsList, adjacencyCosts);
		total = total + (route.vehicle.travelCost * totalTravel);
	}
	return total;
}

float RouteDefs::evaluateRoute(const Route& route, const AdjacencyCosts& adjacencyCosts) {
	return calculateTravelCost(route.clientsList, adjacencyCosts) * route.vehicle.travelCost + route.vehicle.cost;
}

void RouteDefs::printRoute(const Route& route) {
	for (const Client& c : route.clientsList) {
		cout << "(" << c.id << "," << c.demand << "),";
	}
	cout << "\n";
}

void RouteDefs::swapClients(Route& r1, int r1clientId, Route& r2, int r2clientId) {
	// O novo cliente é o cliente da outra rota.
	int oldR1ClientIndex = r1.findClient(r1clientId);
	int oldR2ClientIndex = r2.findClient(r2clientId);
	Client newR1Client(r2.clientsList[oldR2ClientIndex]);
	Client newR2Client(r1.clientsList[oldR1ClientIndex]);
	r1.clientsList[oldR1ClientIndex] = newR1Client;
	r2.clientsList[oldR2ClientIndex] = newR2Client;
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

std::pair<float, int> RouteDefs::findBestInsertion(const Route& route, const std::vector<Client>& clientsList, const AdjacencyCosts& adjacencyCosts, int forbiddenIndex) {
	int firstClientId = clientsList.front().id;
	float firstClientdemand = clientsList.front().demand;
	int lastClientId = clientsList.back().id;
	float lastClientDemand = clientsList.back().demand;
	std::pair<float, int> result;
	float cost = 0;
	int pos = 0;
	// Se colocar na primeira posição.
	cost = adjacencyCosts.depotTravel[firstClientId];
	pos = 0;

	int clients = route.clientsList.size();

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
				float currCost = 0;
				if (i == 0) { // Colocar um cliente no inicio
					currCost = adjacencyCosts.depotTravel[firstClientId] + adjacencyCosts.getAdjacencyCosts(lastClientId, currClientId);
				}
				else if (i == clients) { // Colocar um cliente no fim
					currCost = adjacencyCosts.getAdjacencyCosts(firstClientId, route.clientsList[i - 1].id) + adjacencyCosts.depotTravel[lastClientId];
				}
				else {
					// Consertar
					int prevId = route.clientsList[i - 1].id;
					currCost = adjacencyCosts.getAdjacencyCosts(prevId, firstClientId) + adjacencyCosts.getAdjacencyCosts(lastClientId, currClientId); // Colocar um cliente entre dois é somar as duas viagems que isso envolve.
				}
				if (currCost < cost) {
					cost = currCost;
					pos = i;
				}
			}
		}
		result.first = cost;
		result.second = pos;
		return result;
	}
}

/*
std::pair<float, int> RouteDefs::findBestInsertion(const Route& route, const Client& client, const AdjacencyCosts& adjacencyCosts) {
	int clientIndex = client.id;
	float demand = client.demand;
	std::pair<float, int> result;
	float cost = 0;
	int pos = 0;
	// Se colocar na primeira posição.
	cost = adjacencyCosts.depotTravel[clientIndex];
	pos = 0;

	int clients = route.clientsList.size();

	if (clients == 0) { // Inserir em rota  vazia.
		result.first = cost * 2;
		result.second = pos;
		return result;
	}

	for (int i = 0; i <= clients; i++) {
		int currClientIndex = -1;
		if (i != clients) {
			currClientIndex = route.clientsList[i].id;
		}
		float currCost = 0;
		if (i == 0) { // Colocar um cliente no inicio
			currCost = adjacencyCosts.depotTravel[clientIndex] + adjacencyCosts.getAdjacencyCosts(clientIndex, currClientIndex);
		}
		else if (currClientIndex == -1) { // Colocar um cliente no fim
			currCost = adjacencyCosts.getAdjacencyCosts(clientIndex, route.clientsList[i - 1].id) + adjacencyCosts.depotTravel[clientIndex];
		}
		else {
			int prevIndex = currClientIndex - 1;
			currCost = adjacencyCosts.getAdjacencyCosts(prevIndex, clientIndex) + adjacencyCosts.getAdjacencyCosts(clientIndex, currClientIndex); // Colocar um cliente entre dois é somar as duas viagems que isso envolve.
		}
		if (currCost < cost) {
			cost = currCost;
			pos = i;
		}
	}
	result.first = cost;
	result.second = pos;
	return result;
}

std::pair<float, int> RouteDefs::findBestInsertion(Route& route, Client& client, Client& next, const AdjacencyCosts& adjacencyCosts) {
	int clientIndex = client.id;
	float demand = client.demand;
	int nextIndex = next.id;
	float nextDemand = next.demand;
	std::pair<float, int> result;
	float cost = 0;
	int pos = 0;
	// Se colocar na primeira posição.
	cost = adjacencyCosts.depotTravel[clientIndex];
	pos = 0;

	int clients = route.clientsList.size();

	if (clients == 0) { // Inserir em rota  vazia.
		result.first = cost + adjacencyCosts.depotTravel[nextIndex];
		result.second = pos;
		return result;
	}

	for (int i = 0; i <= clients; i++) {
		int currClientIndex = -1;
		if (i != clients) {
			currClientIndex = route.clientsList[i].id;
		}
		float currCost = 0;
		if (i == 0) { // Colocar um cliente no inicio
			currCost = adjacencyCosts.depotTravel[clientIndex] + adjacencyCosts.getAdjacencyCosts(nextIndex, currClientIndex);
		}
		else if (currClientIndex == -1) { // Colocar um cliente no fim
			currCost = adjacencyCosts.getAdjacencyCosts(clientIndex, route.clientsList[i - 1].id) + adjacencyCosts.depotTravel[nextIndex];
		}
		else {
			// Consertar
			int prevIndex = currClientIndex - 1;
			currCost = adjacencyCosts.getAdjacencyCosts(prevIndex, clientIndex) + adjacencyCosts.getAdjacencyCosts(nextIndex, currClientIndex); // Colocar um cliente entre dois é somar as duas viagems que isso envolve.
		}
		if (currCost < cost) {
			cost = currCost;
			pos = i;
		}
	}
	result.first = cost;
	result.second = pos;
	return result;
}
*/

std::vector<Route> RouteDefs::copy(const std::vector<Route>& other) {
	std::vector<Route> result;
	result.reserve(other.size());
	for (int i = 0; i < other.size(); i++) {
		result.push_back(Route(other[i]));
	}
	return result;
}

bool RouteDefs::isSolutionValid(const std::vector<Route>& solution, std::vector<Client> completeClientList) {
	std::vector<int> demandsApplied;
	for (int i = 0; i < completeClientList.size(); i++) {
		demandsApplied.push_back(completeClientList[i].demand);
	}
	for (int i = 0; i < solution.size(); i++) {
		if (solution[i].getTotalDemand() > solution[i].vehicle.capacity) {
			return false;
		}
		std::vector<int> idsVisited; // Ver se não tem o mesmo cliente duas vezes.
		for (int j = 0; j < solution[i].clientsList.size(); j++) {
			int id = solution[i].clientsList[j].id;
			for (int idVisited : idsVisited) {
				if (idVisited == id) {
					return false;
				}
			}
			idsVisited.push_back(id);
			demandsApplied[id] -= solution[i].clientsList[j].demand;
		}
	}

	for (int i = 0; i < completeClientList.size(); i++) {
		if (demandsApplied[i] > 0 || demandsApplied[i] < 0) {
			return false;
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

bool RouteDefs::fitsInNonBiggestVehicle(int demand, const std::vector<Vehicle>& vehiclesList) {
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