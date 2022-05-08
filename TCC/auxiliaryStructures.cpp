#include "auxiliaryStructures.h"
#include <iostream>
#include <stdio.h> 
#include <functional>

using namespace std;

AuxiliaryStructures::AuxiliaryStructures() {

}

AuxiliaryStructures::AuxiliaryStructures(const std::vector<std::vector<int>>& adjacencies, int neighborhoodTypes) {
	m_neighborhoodTypes = neighborhoodTypes;
	m_adjacencyMatrix = &adjacencies;
}

AuxiliaryStructures::~AuxiliaryStructures() {

}

void AuxiliaryStructures::recalculate(const std::vector<Route>& routes)
{
	calculateCumulativeDelivery(routes);
	calculateMaxDelivery(routes);
	calculateMaxPairDelivery(routes);
	calculateMinDelivery(routes);
	calculateMinPairDelivery(routes);
	calculateNeighborhoodStatus(routes);
	calculateSumDelivery(routes);
}

int AuxiliaryStructures::sumDelivery(int route) const
{
	return m_sumDelivery[route];
}

int AuxiliaryStructures::minDelivery(int route) const
{
	return m_minDelivery[route];
}

int AuxiliaryStructures::maxDelivery(int route) const
{
	return m_maxDelivery[route];
}

int AuxiliaryStructures::minPairDelivery(int route) const
{
	return m_minPairDelivery[route];
}

int AuxiliaryStructures::maxPairDelivery(int route) const
{
	return m_maxPairDelivery[route];
}

int AuxiliaryStructures::cumulativeDelivery(int routeIndex, int customers) const
{
	int total = 0;
	int size = m_cumulativeDelivery[routeIndex].size();
	if (customers > size) {
		cout << "AuxiliaryStructures::cumulativeDelivery , pedido de clientes maior que quantidade de clientes na rota";
		exit(1);
	}
	for (int i = 0; i < customers && i < size; i++) {
		total += m_cumulativeDelivery[routeIndex][i];
	}
	return total;
}

bool AuxiliaryStructures::neighborhoodStatus(NEIGHBORHOODTYPES neighborhoodtype, int route) const
{
	int rel = (int) neighborhoodtype;
	auto neigh = m_neighborhoodStatus[rel].at(route);
	return neigh.improvementStatus && neigh.routeAltered;
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
			int demandsListSize = r.demands.size();
			for (int iClient = 0; iClient < demandsListSize; iClient++) {
				sum += r.demands[iClient];
			}
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
			int demandsListSize = r.demands.size();
			for (int iClient = 0; iClient < demandsListSize; iClient++) {
				int currDemand = r.demands[iClient];
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
			int demandsListSize = r.demands.size();
			for (int iClient = 0; iClient < demandsListSize; iClient++) {
				int currDemand = r.demands[iClient];
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
		const std::vector<int> clientsInRoute = routes[iRoute].clientsList;
		for (int iClient1 = 0; iClient1 < clientsInRoute.size(); iClient1++) {
			int currClient1 = clientsInRoute[iClient1];
			for (int iClient2 = 0; iClient2 < clientsInRoute.size(); iClient2++) {
				int currClient2 = clientsInRoute[iClient2];
				auto client1Adjacencies = m_adjacencyMatrix->at(currClient1);
				if (client1Adjacencies.at(currClient2) > 0) {
					int pairDemand = routes[iRoute].demands[currClient1] + routes[iRoute].demands[currClient2];
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
		const std::vector<int> clientsInRoute = routes[iRoute].clientsList;
		for (int iClient1 = 0; iClient1 < clientsInRoute.size(); iClient1++) {
			for (int iClient2 = 0; iClient2 < clientsInRoute.size(); iClient2++) {
				auto client1Adjacencies = m_adjacencyMatrix->at(iClient1);
				if (client1Adjacencies.at(iClient2) > 0) {
					int pairDemand = routes[iRoute].demands[iClient1] + routes[iRoute].demands[iClient2];
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
	int total = 0;
	int routeSize = routes.size();
	m_cumulativeDelivery.clear();
	m_cumulativeDelivery.reserve(routeSize);
	for (int iRoute = 0; iRoute < routeSize; iRoute++) {
		int demandsSize = routes[iRoute].demands.size();
		std::vector<int> demand;
		demand.reserve(demandsSize);
		for (int iDemand = 0; iDemand < demandsSize; iDemand++) {
			total += routes[iRoute].demands[iDemand];
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

void AuxiliaryStructures::routeAltered(NEIGHBORHOODTYPES neighborhoodType, int routeId) {
	int typeInt = (int)neighborhoodType;
		m_neighborhoodStatus[typeInt][routeId].routeAltered = true;
}

void AuxiliaryStructures::improvementChanged(NEIGHBORHOODTYPES neighborhoodType, int route, bool change) {
	int typeInt = (int)neighborhoodType;
	m_neighborhoodStatus[typeInt][route].improvementStatus = change;
}