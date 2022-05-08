#include "heterogeneousTransports.h"
#include "Utils.h"

using namespace std;

PRVFHEF::PRVFHEF(std::vector<int> clientsDemands, std::vector<ClientAdjacency> clientAdjacencies, std::vector<Vehicle> vehicleTypes, int vehicles) {
	m_clientsCount = clientsDemands.size();
	m_allClients.resize(m_clientsCount);
	m_vehicleTypes = vehicleTypes;
	createAdjacencyMatrix(m_clientsCount, clientAdjacencies);
	for (auto i = 0; i < clientsDemands.size(); i++) {
		Client c;
		c.id = i;
		c.demand = clientsDemands[i];
		m_allClients[i] = c;
	}
	execute(vehicles, 10, 5);
}

PRVFHEF::~PRVFHEF() {

}

Vehicle PRVFHEF::getBiggestVehicle() const {
	Vehicle biggestVehicle = m_vehicleTypes[0];
	for (auto iVels = 1; iVels < m_vehicleTypes.size(); iVels++) {
		if (m_vehicleTypes[iVels].capacity > biggestVehicle.capacity) {
			biggestVehicle = m_vehicleTypes[iVels];
		}
	}
	return biggestVehicle;
}

int PRVFHEF::estimateVehicles(const std::vector<Client>& allClients) const {
	Vehicle biggestVehicle = getBiggestVehicle();

	int totalDemands = 0;
	for (auto iCli = 0; iCli < allClients.size(); iCli++) {
		totalDemands += allClients[iCli].demand;
	}
	
	return (1 + ((totalDemands - 1) / biggestVehicle.capacity));
}

std::vector<Route> PRVFHEF::createInitialSolution(int vehiclesCount) {
	std::list<int> candidates = initializeCandidatesList(m_allClients);
	std::vector<Route> routes;
	int candidatesLeft = candidates.size();
	for (auto iVel = 0; iVel < vehiclesCount; iVel++) {
		Route r = this->m_routeCreator.createRoute();
		r.vehicle = getBiggestVehicle();
		int candidateIndex = Utils::getRandomInt(0, candidates.size());
		int candidateId;
		auto iterator = candidates.begin();
		std::advance(iterator, candidateIndex);
		candidateId = *iterator;
		r.addClient(candidateId, m_allClients[candidateId].demand);
		candidates.remove(candidateIndex);
		candidatesLeft -= 1;
		routes.push_back(r);
	}

	paralelInsertion(routes, candidates, 0);

	cout << routes.size() << '\n'; 

	// Adição de rotas vazias para o caso de frota ilimitada
	for (int i = 0; i < m_vehicleTypes.size(); i++) {
		Route emptyVelType = m_routeCreator.createRoute();
		emptyVelType.vehicle = m_vehicleTypes[i];
		routes.push_back(emptyVelType);
	}

	return routes;
}

std::list<int> PRVFHEF::initializeCandidatesList(const std::vector<Client>& allClients) const {
	std::list<int> list;
	for (auto i = 0; i < allClients.size(); i++) {
		list.push_back(allClients[i].id);
	}
	return list;
}

void PRVFHEF::createAdjacencyMatrix(int clientsCount, std::vector<ClientAdjacency>& clientAdjacencies) {
	// Custo de cliente a -> b = a * clientCount + b 
	m_adjacencyCosts = std::vector<std::vector<int>>(clientsCount);
	for (auto i = 0; i < clientsCount; i++) {
		m_adjacencyCosts[i] = std::vector<int>(m_clientsCount);
		m_adjacencyCosts[i][0] = 0;
	}
	for (auto j = 0; j < clientAdjacencies.size(); j++) {
		ClientAdjacency& curr = clientAdjacencies[j];
		int id1 = curr.clientFromId;
		int id2 = curr.clientToId;
		m_adjacencyCosts[id1][id2] = curr.travelCost;
	}
}

std::vector<Route> PRVFHEF::rvnd(std::vector<Route>& currSol) {
	// Cria lista de vizinhança
	m_auxiliaryStructures.recalculate(currSol);
	std::list<INTERROUTETYPES> neighborhoodList = interrouteStructures::getAll();
	std::vector<Route> finalResult = currSol;
	// Atualizar estruturas de dados auxiliares
	int interroutesLeft = neighborhoodList.size();
	while (!neighborhoodList.empty()) {
		// Escolhe aleatóriamente uma vizinhança.
		std::list<INTERROUTETYPES>::iterator iterator = neighborhoodList.begin();

		int selectedInterrouteIndex = Utils::getRandomInt(0, neighborhoodList.size());
		std::advance(iterator, selectedInterrouteIndex);
		INTERROUTETYPES selectedInterroute = *iterator;

		// Execução da interrota.
		std::vector<Route> roundResult = finalResult;
		roundResult = interrouteStructures::executeInterroute(selectedInterroute, roundResult);
		
		// Remoção da vizinhança.
		neighborhoodList.remove(selectedInterroute);
		interroutesLeft -= 1;
		m_auxiliaryStructures.recalculate(currSol);
	}
	return finalResult;
}

void PRVFHEF::execute(int initialVehicles, int iterations, int maxItersNoImprove) {
	int vehicles = initialVehicles;
	if (vehicles == -1) {
		vehicles = estimateVehicles(m_allClients);
	}
	int eval = INT_MAX;
	for (int algItrs = 0; algItrs < iterations; algItrs) {
		std::vector<Route> initialSolution = createInitialSolution(vehicles);
		m_auxiliaryStructures = AuxiliaryStructures(m_adjacencyCosts, interrouteStructures::getAll().size());
		std::vector<Route> solutionOptimized = rvnd(initialSolution);
		for (int ilsIters = 0; ilsIters < maxItersNoImprove; ilsIters++) {
			perturbationMethods::perturbate(solutionOptimized);
			//solutionOptimized = perturbationMethods::perturbate(solutionOptimized);
		}
	}
}

int PRVFHEF::evaluate(std::vector<Route> solution) const {
	return 0;
}

void PRVFHEF::paralelInsertion(std::vector<Route>& routes, std::list<int>& candidatesList, bool insertionCriteria) {
	while (candidatesList.size() != 0) {
		int candidateIndex = 0;
		while (candidateIndex < candidatesList.size()) {
			auto itCandidate = candidatesList.begin();
			advance(itCandidate, candidateIndex);
			int currentCandidateId = *itCandidate;
			int closestRouteIndex = -1;
			int bestClosestCost = INT_MAX;
			bool canInsert = false;
			for (int iRoute = 0; iRoute < routes.size(); iRoute++) {
				bool currCanInsert = routes[iRoute].canAddClient(m_allClients[currentCandidateId].demand);
				if (currCanInsert) {
					canInsert = true;
					int cost = getClosestInsertionCost(routes[iRoute], currentCandidateId);
					if (cost < bestClosestCost) {
						bestClosestCost = cost;
						closestRouteIndex = iRoute;
					}
				}
			}
			if (canInsert) {
				routes[closestRouteIndex].addClient(currentCandidateId, m_allClients[currentCandidateId].demand);
				int prevSize = candidatesList.size();
				candidatesList.remove(currentCandidateId);
			}
			else {
				candidateIndex++;
			}
		}
		if (candidatesList.size() > 0) {
			int randomVehicleType = Utils::getRandomInt(0, m_vehicleTypes.size());
			Vehicle randomVehicle = m_vehicleTypes[randomVehicleType];
			Route routeToAdd = m_routeCreator.createRoute();
			routeToAdd.vehicle = randomVehicle;
			routes.push_back(routeToAdd);
		}
	}
	
}

int PRVFHEF::getClosestInsertionCost(const Route& route, int candidateId) const {
	//int cost = INT_MAX;
	int cost = 0;
	for (int iCand = 0; iCand < route.clientsList.size(); iCand++) {
		int currClientId = route.clientsList[iCand];
		int currCost = getAdjacencyCosts(candidateId, currClientId);
		/*
		if (currCost < cost) {
			cost = currCost;
		}
		*/
		cost += currCost;
	}
	return cost;
}

int PRVFHEF::getAdjacencyCosts(int client1, int client2) const {
	return m_adjacencyCosts[client1][client2];
}

std::vector<Route> PRVFHEF::splitReinsertion(const std::vector<Route>& solution, int clientIndex, int clientDemand, int forbiddenRouteId) {
	std::vector<int> routesIndexWithResidual; // L
	std::vector<int> residualsList; // A
	std::vector<int> leastInsertionCosts; // U

	int vehicles = solution.size();
	int totalResidual = 0;
	for (int i = 0; i < vehicles; i++) {
		Route& r = solution[i];
		int currResidual = r.vehicle.capacity - r.getTotalDemand();
		if (r.id != forbiddenRouteId && currResidual > 0) {
			routesIndexWithResidual.push_back(i);
			residualsList.push_back(currResidual);
			totalResidual += currResidual;
		}
	}

	if (totalResidual > clientDemand) { // Então existe o espaço para o cliente.
		for (int i = 0; i < routesIndexWithResidual.size(); i++) {
			int leastCostToInsert;
			leastInsertionCosts.push_back(leastCostToInsert);
		}
	}

	return knaapSackGreedy(solution, clientIndex, clientDemand, routesIndexWithResidual, residualsList, leastInsertionCosts);
}

std::vector<Route> PRVFHEF::knaapSackGreedy(const std::vector<Route>& solution, int clientIndex, int clientDemand,
	std::vector<int>& routesIndexWithResidual, std::vector<int>& residualsList, std::vector<int>& leastInsertionCosts) {



}

