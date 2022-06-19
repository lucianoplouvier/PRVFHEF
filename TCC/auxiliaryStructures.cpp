#include "auxiliaryStructures.h"
#include <iostream>
#include <stdio.h> 
#include <functional>

using namespace std;
#include "interrouteStructures.h"

AuxiliaryStructures::AuxiliaryStructures(const AdjacencyCosts* adjacenciesCosts, int neighborhoodTypes) {
	m_neighborhoodTypes = neighborhoodTypes;
	m_adjacencyMatrix = adjacenciesCosts;
}

AuxiliaryStructures::~AuxiliaryStructures() {

}

void AuxiliaryStructures::recalculate(const std::vector<Route>& routes)
{
	return; // TODO CONSERTAR
	calculateCumulativeDelivery(routes);
	calculateMaxDelivery(routes);
	calculateMaxPairDelivery(routes);
	calculateMinDelivery(routes);
	calculateMinPairDelivery(routes);
	calculateNeighborhoodStatus(routes);
	calculateSumDelivery(routes);
}

float AuxiliaryStructures::sumDelivery(int routeIndex) const
{
	return m_sumDelivery[routeIndex];
}

float AuxiliaryStructures::minDelivery(int routeIndex) const
{
	return m_minDelivery[routeIndex];
}

float AuxiliaryStructures::maxDelivery(int routeIndex) const
{
	return m_maxDelivery[routeIndex];
}

float AuxiliaryStructures::minPairDelivery(int routeIndex) const
{
	return m_minPairDelivery[routeIndex];
}

float AuxiliaryStructures::maxPairDelivery(int routeIndex) const
{
	return m_maxPairDelivery[routeIndex];
}

float AuxiliaryStructures::cumulativeDelivery(int routeIndex, int customers) const
{
	int total = 0;
	int size = m_cumulativeDelivery[routeIndex].size();
	if (customers > size) {
		cout << "AuxiliaryStructures::cumulativeDelivery , pedido de clientes maior que quantidade de clientes na rota";
		customers = size;
	}
	for (int i = 0; i < customers && i < size; i++) {
		total += m_cumulativeDelivery[routeIndex][i];
	}
	return total;
}

bool AuxiliaryStructures::neighborhoodStatus(INTERROUTETYPES neighborhoodtype, int routeId) const
{
	/*
	* TODO consertar isso aqui
	int rel = (int) neighborhoodtype;
	auto neigh = m_neighborhoodStatus[rel].at(routeId);
	return neigh.improvementStatus && neigh.routeAltered;
	*/
	return true;
}

void AuxiliaryStructures::calculateSumDelivery(const std::vector<Route>& routes)
{
	int routeSize = routes.size();
	m_sumDelivery.clear();
	if (routeSize > 0) {
		m_sumDelivery.reserve(routeSize);
		for (int iRoute = 0; iRoute < routeSize; iRoute++) {
			const Route r = routes[iRoute];
			int sum = 0;
			int demandsListSize = r.clientsList.size();
			sum = r.getTotalDemand();
			m_sumDelivery.push_back(sum);
		}
	}
}

void AuxiliaryStructures::calculateMinDelivery(const std::vector<Route>& routes)
{
	int routeSize = routes.size();
	m_minDelivery.clear();
	if (routeSize > 0) {
		m_minDelivery.reserve(routeSize);
		for (int iRoute = 0; iRoute < routeSize; iRoute++) {
			const Route r = routes[iRoute];
			int minDel = INT_MAX;
			int demandsListSize = r.clientsList.size();
			for (int iClient = 0; iClient < demandsListSize; iClient++) {
				int currDemand = r.clientsList[iClient].demand;
				if (currDemand < minDel) {
					minDel = currDemand;
				}
			}
			m_minDelivery.push_back(minDel);
		}
	}
}

void AuxiliaryStructures::calculateMaxDelivery(const std::vector<Route>& routes)
{
	int routeSize = routes.size();
	m_maxDelivery.clear();
	if (routeSize > 0) {
		m_maxDelivery.reserve(routeSize);
		for (int iRoute = 0; iRoute < routeSize; iRoute++) {
			const Route r = routes[iRoute];
			int maxDel = INT_MIN;
			int demandsListSize = r.clientsList.size();
			for (int iClient = 0; iClient < demandsListSize; iClient++) {
				int currDemand = r.clientsList[iClient].demand;
				if (currDemand > maxDel) {
					maxDel = currDemand;
				}
			}
			m_maxDelivery.push_back(maxDel);
		}
	}
}

void AuxiliaryStructures::calculateMinPairDelivery(const std::vector<Route>& routes)
{
	int routesSize = routes.size();
	m_minPairDelivery.clear();
	m_minPairDelivery.reserve(routes.size());
	for (int iRoute = 0; iRoute < routesSize; iRoute++) {
		int routeLeastSum = INT_MAX;
		const std::vector<Client> clientsInRoute = routes[iRoute].clientsList;
		for (int iClient1 = 0; iClient1 < clientsInRoute.size(); iClient1++) {
			int currClient1Id = clientsInRoute[iClient1].id;
			for (int iClient2 = 0; iClient2 < clientsInRoute.size(); iClient2++) {
				int currClient2Id = clientsInRoute[iClient2].id;
				auto client1Adjacencies = m_adjacencyMatrix->costs.at(currClient1Id);
				if (client1Adjacencies.at(currClient2Id) > 0) {
					int pairDemand = routes[iRoute].clientsList[iClient1].demand + routes[iRoute].clientsList[iClient2].demand;
					if (pairDemand < routeLeastSum) {
						routeLeastSum = INT_MAX;
					}
				}
			}
		}
		m_minPairDelivery.push_back(routeLeastSum);
	}
}

void AuxiliaryStructures::calculateMaxPairDelivery(const std::vector<Route>& routes)
{
	int routesSize = routes.size();
	m_maxPairDelivery.clear();
	m_maxPairDelivery.reserve(routes.size());
	for (int iRoute = 0; iRoute < routesSize; iRoute++) {
		int routeLargestSum = INT_MAX;
		const std::vector<Client> clientsInRoute = routes[iRoute].clientsList;
		for (int iClient1 = 0; iClient1 < clientsInRoute.size(); iClient1++) {
			for (int iClient2 = 0; iClient2 < clientsInRoute.size(); iClient2++) {
				auto client1Adjacencies = m_adjacencyMatrix->costs.at(iClient1);
				if (client1Adjacencies.at(iClient2) > 0) {
					int pairDemand = routes[iRoute].clientsList[iClient1].demand + routes[iRoute].clientsList[iClient2].demand;
					if (pairDemand > routeLargestSum) {
						routeLargestSum = INT_MAX;
					}
				}
			}
		}
		m_maxPairDelivery.push_back(routeLargestSum);
	}
}

void AuxiliaryStructures::calculateCumulativeDelivery(const std::vector<Route>& routes)
{
	float total = 0;
	int routeSize = routes.size();
	m_cumulativeDelivery.clear();
	m_cumulativeDelivery.reserve(routeSize);
	for (int iRoute = 0; iRoute < routeSize; iRoute++) {
		int demandsSize = routes[iRoute].clientsList.size();
		std::vector<float> demand;
		demand.reserve(demandsSize);
		for (int iDemand = 0; iDemand < demandsSize; iDemand++) {
			total += routes[iRoute].clientsList[iDemand].demand;
			demand.push_back(total);
		}
		m_cumulativeDelivery.push_back(demand);
	}
}

void AuxiliaryStructures::calculateNeighborhoodStatus(const std::vector<Route>& routes)
{
	int types = m_neighborhoodTypes;

	std::vector<std::map<int, neighborhoodStatusNode>> newStatus;
	newStatus.reserve(types);
	for (int i = 0; i < types; i++) {
		newStatus.push_back(std::map<int, neighborhoodStatusNode>());
		int j = 0;
		while (j < routes.size()) {
			int routeId = routes[j].id;
			bool improvementStatus = true;
			bool routeAltered = false;
			if (m_neighborhoodStatus.size()) {
				auto iterator = m_neighborhoodStatus[i].find(routeId);
				if (iterator != m_neighborhoodStatus[i].end()) {
					routeAltered = iterator->second.routeAltered;
					improvementStatus = iterator->second.improvementStatus;
				}
			}
			neighborhoodStatusNode n;
			n.improvementStatus = improvementStatus;
			n.routeAltered = routeAltered;
			newStatus[i][routeId] = n;
			j++;
		}
	}
	m_neighborhoodStatus = newStatus;
}

void AuxiliaryStructures::routeAltered(INTERROUTETYPES neighborhoodType, int routeId, bool alteration) {
	int typeInt = (int)neighborhoodType;
	m_neighborhoodStatus[typeInt][routeId].routeAltered = alteration;
}

void AuxiliaryStructures::improvementChanged(INTERROUTETYPES neighborhoodType, int route, bool change) {
	int typeInt = (int)neighborhoodType;
	m_neighborhoodStatus[typeInt][route].improvementStatus = change;
}