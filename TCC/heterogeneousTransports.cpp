#include "heterogeneousTransports.h"
#include "Utils.h"

using namespace std;

PRVFHEF::PRVFHEF(std::vector<float> clientsDemands, std::vector<ClientAdjacency> clientAdjacencies, std::vector<Vehicle> vehicleTypes, std::vector<float> depotTravelCost, int vehicles) {
	m_clientsCount = clientsDemands.size();
	m_allClients.resize(m_clientsCount);
	m_vehicleTypes = vehicleTypes;
	createAdjacencyMatrix(m_clientsCount, clientAdjacencies, depotTravelCost);
	for (auto i = 0; i < clientsDemands.size(); i++) {
		Client c;
		c.id = i;
		c.demand = clientsDemands[i];
		m_allClients[i] = c;
	}
	m_clientsOriginalDemands = clientsDemands;
	m_auxiliaryStructures = NULL;
	execute(vehicles, 100, 50);
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

	float totalDemands = 0;
	for (auto iCli = 0; iCli < allClients.size(); iCli++) {
		totalDemands += allClients[iCli].demand;
	}
	
	return (1 + ((totalDemands - 1) / biggestVehicle.capacity));
}

void PRVFHEF::addClientToRoute(Route& r, int candidateId, float demandAmount) {
	if (demandAmount > m_allClients[candidateId].demand) {
		cout << "WARN . PRVFHEF::addClientToRoute, Demanda a tirar do client maior do que a demanda que ele tem.";
	}
	r.addClient(candidateId, m_allClients[candidateId].demand);
	m_allClients[candidateId].demand -= demandAmount;
}

std::vector<Route> PRVFHEF::createInitialSolution(int vehiclesCount) {
	std::list<int> candidates = initializeCandidatesList(m_allClients);
	std::vector<Route> routes;
	int candidatesLeft = candidates.size();
	for (auto iVel = 0; iVel < vehiclesCount; iVel++) {
		Route r = this->m_routeCreator.createRoute(m_vehicleTypes[iVel]);
		int candidateIndex = Utils::getRandomInt(0, candidates.size());
		int candidateId;
		auto iterator = candidates.begin();
		std::advance(iterator, candidateIndex);
		candidateId = *iterator;
		bool added = r.addClient(candidateId, m_allClients[candidateId].demand); // Ignorar Rotas Fracionárias
		if (added) {
			candidates.remove(candidateId);
			candidatesLeft -= 1;
			routes.push_back(r);
		}
	}

	routes = paralelInsertion(routes, candidates, 0);

	int countClients = 0;
	for (Route r : routes) {
		countClients += r.clientsList.size();
	}
	if (countClients != m_clientsCount) {
		int ashudas = 0;
	}

	cout << "Initial Routes: " << routes.size() << '\n'; 

	// Adição de rotas vazias para o caso de frota ilimitada
	for (int i = 0; i < m_vehicleTypes.size(); i++) {
		Route emptyVelType = m_routeCreator.createRoute(m_vehicleTypes[i]);
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

void PRVFHEF::createAdjacencyMatrix(int clientsCount, std::vector<ClientAdjacency>& clientAdjacencies, std::vector<float> depotTravelCost) {
	// Custo de cliente a -> b = a * clientCount + b 
	AdjacencyCosts adjCosts;
	adjCosts.depotTravel = depotTravelCost;
	adjCosts.costs.reserve(clientsCount);
	m_adjacencyCosts = adjCosts;
	for (auto i = 0; i < clientsCount; i++) {
		m_adjacencyCosts.costs.push_back(std::vector<float>(m_clientsCount));
		m_adjacencyCosts.costs[i][i] = 0;
	}
	for (auto j = 0; j < clientAdjacencies.size(); j++) {
		ClientAdjacency& curr = clientAdjacencies[j];
		int id1 = curr.clientFromId;
		int id2 = curr.clientToId;
		m_adjacencyCosts.costs[id1][id2] = curr.travelCost;
		m_adjacencyCosts.costs[id2][id1] = curr.travelCost;
	}
}

std::vector<Route> PRVFHEF::intraroute(const std::vector<Route>& solution, float evaluation) {
	std::list<INTRAROUTETYPES> intrarouteList = intrarouteStructures::getAll();
	std::vector<Route> result = RouteDefs::copy(solution);
	float bestEval = evaluation;
	while (!intrarouteList.empty()) {
		std::list<INTRAROUTETYPES>::iterator iterator = intrarouteList.begin();
		int selectedIntrarouteIndex = Utils::getRandomInt(0, intrarouteList.size());
		std::advance(iterator, selectedIntrarouteIndex);
		INTRAROUTETYPES selectedIntraroute = *iterator;
		std::vector<Route> roundResult = intrarouteStructures::execute(selectedIntraroute, result, m_auxiliaryStructures, m_adjacencyCosts);
		float eval = RouteDefs::evaluate(roundResult, m_adjacencyCosts);
		if (eval < bestEval) {
			bestEval = eval;
			result = roundResult;
		}
		else {
			intrarouteList.remove(selectedIntraroute);
		}
	}
	return result;
}

std::vector<Route> PRVFHEF::rvnd(std::vector<Route>& currSol, float evaluation) {
	// Cria lista de vizinhança
	m_auxiliaryStructures->recalculate(currSol);
	std::list<INTERROUTETYPES> neighborhoodList = interrouteStructures::getAll();
	std::vector<Route> finalResult = RouteDefs::copy(currSol);
	float finalEval = evaluate(finalResult);
	// Atualizar estruturas de dados auxiliares
	int interroutesLeft = neighborhoodList.size();
	while (!neighborhoodList.empty()) {
		// Escolhe aleatóriamente uma vizinhança.
		std::list<INTERROUTETYPES>::iterator iterator = neighborhoodList.begin();

		int selectedInterrouteIndex = Utils::getRandomInt(0, neighborhoodList.size());
		std::advance(iterator, selectedInterrouteIndex);
		INTERROUTETYPES selectedInterroute = *iterator;

		// Execução da interrota.
		std::vector<Route> roundResult = RouteDefs::copy(finalResult);
		std::vector<Route> interrouteResult = interrouteStructures::executeInterroute(selectedInterroute, roundResult, evaluation, m_auxiliaryStructures, m_adjacencyCosts, m_allClients);
		float roundEval = evaluate(interrouteResult);
		if (roundEval < finalEval) {
			finalResult = intraroute(interrouteResult, roundEval);
			finalEval = RouteDefs::evaluate(finalResult, m_adjacencyCosts);
			neighborhoodList = interrouteStructures::getAll();
			// Reiniciar a lista de vizinhanças.
		}
		else {
			// Remoção da vizinhança.
			neighborhoodList.remove(selectedInterroute);
			interroutesLeft -= 1;
		}
		m_auxiliaryStructures->recalculate(currSol);
	}
	return finalResult;
}

void PRVFHEF::execute(int initialVehicles, int iterations, int maxItersNoImprove) {
	int vehicles = initialVehicles;
	if (vehicles == -1) {
		vehicles = estimateVehicles(m_allClients);
	}
	std::vector<Route> finalSolution;
	float finalEval = std::numeric_limits<float>::max();
	for (int algItrs = 0; algItrs < iterations; algItrs) {
		// Executa cada rodada do algoritmo, onde uma nova solução é procurada a partir do 0.
		m_auxiliaryStructures = new AuxiliaryStructures(&m_adjacencyCosts, interrouteStructures::getAll().size());
		std::vector<Route> solution = createInitialSolution(vehicles);
		float oldEval = evaluate(solution);
		std::vector<Route> solutionOptimized = rvnd(solution, oldEval);
		oldEval = evaluate(solutionOptimized);
		finalSolution = solutionOptimized;
		for (int ilsIters = 0; ilsIters < maxItersNoImprove; ilsIters++) {
			perturbationMethods::perturbate(solutionOptimized);
			m_auxiliaryStructures->recalculate(solutionOptimized);
			solutionOptimized = rvnd(solutionOptimized, oldEval);
			float evaluation = evaluate(solutionOptimized);
			if (evaluation < oldEval) {
				oldEval = evaluation;
				solution = RouteDefs::copy(solutionOptimized);
				ilsIters = -1;
			}
		}
		if (oldEval < finalEval) {
			float redimensionedEval = oldEval;
			vehicleRedimension(solution, oldEval, redimensionedEval);
			finalEval = redimensionedEval;
			finalSolution = RouteDefs::copy(solution);
		}
		delete m_auxiliaryStructures;
		m_auxiliaryStructures = NULL;
	}
	cout << "FINAL: \n";
	if (verifySolutionValid(finalSolution)) {
		printSolution(finalEval, finalSolution);
	}
	else {
		cout << "Erro no final!\n";
	}
}

bool PRVFHEF::verifySolutionValid(const std::vector<Route>& solution) {
	std::vector<float> clientDemands;
	bool error = true;
	for (int i = 0; i < m_allClients.size(); i++) {
		clientDemands.push_back(m_allClients[i].demand);
	}

	for (int i = 0; i < solution.size(); i++) {
		float velCap = solution[i].vehicle.capacity;
		float totalDemand = solution[i].getTotalDemand();
		float rest = velCap - totalDemand;
		if (rest < 0) {
			cout << "ERROR. Usando mais espaco do veiculo que aceitavel. " << velCap << " - " << totalDemand << " = " << rest << "\n";
			error = false;
		}
		else {
			for (int j = 0; j < solution[i].clientsList.size(); j++) {
				int clientId = solution[i].clientsList[j].id;
				clientDemands[clientId] -= solution[i].clientsList[j].demand;
			}
		}
		cout << "\n";
	}
	for (int i = 0; i < clientDemands.size(); i++) {
		if (clientDemands[i] > 0) {
			cout << "ERROR. Cliente de id" << i << " nao esta sendo totalmente atendido. Sobrou: " << clientDemands[i];
			error = false;
		}
		else if (clientDemands[i] < 0) {
			cout << "ERROR. Cliente de id" << i << " com demanda negativa. Valor: " << clientDemands[i];
			error = false;
		}
	}
	return error;
}

void PRVFHEF::printSolution(float eval, const  std::vector<Route>& solution) {
	cout << "\n--------------------------\n";
	cout << "Resultado iteracao. EVAL: " << eval << "\n";
	cout << "Rota: ";
	for (int i = 0; i < solution.size(); i++) {
		if (solution[i].clientsList.size() > 0) {
			const Route& r = solution[i];
			float velCap = r.vehicle.capacity;
			float totalDemand = r.getTotalDemand();
			float rest = velCap - totalDemand;
			if (rest < 0) {
				cout << "ERROR. Usando mais espaço do veiculo que aceitavel. " << velCap << " - " << totalDemand << " = " << rest << "\n";
			}
			else {
				cout << "Veiculo: " << solution[i].vehicle.id << " Capacidade: " << velCap << "\n";
				cout << "Clientes:\n";
				for (int j = 0; j < solution[i].clientsList.size(); j++) {
					cout << "(I:" << solution[i].clientsList[j].id << ", D:" << solution[i].clientsList[j].demand << ")";
					if (j + 1 < solution[i].clientsList.size()) {
						cout << ", ";
					}
					cout << "\nSobrou do veiculo: " << rest << "\n";
				}
				cout << "\n";
			}
		}
	}
}

float PRVFHEF::evaluate(std::vector<Route>& solution) const {
	return RouteDefs::evaluate(solution, m_adjacencyCosts);
}

bool PRVFHEF::canAddAnyToARoute(const std::vector<Route>& routes, const std::list<int>& candidatesList) {
	for (int i = 0; i < routes.size(); i++) {
		for (int j = 0; j < candidatesList.size(); j++) {
			auto itCandidate = candidatesList.begin();
			advance(itCandidate, j);
			if (routes[i].canAddClient(m_allClients[*itCandidate].demand)) {
				return true;
			}
		}
	}
	return false;
}

std::vector<Route> PRVFHEF::paralelInsertion(std::vector<Route>& routes, std::list<int>& candidatesList, bool insertionCriteria) {
	std::vector<Route> result;
	while (candidatesList.size() > 0) {
		int candidateIndex = 0;
		while (candidatesList.size() > 0 && canAddAnyToARoute(routes, candidatesList)) {
			auto itCandidate = candidatesList.begin();
			advance(itCandidate, candidateIndex);
			int currentCandidateId = *itCandidate;
			int closestRouteIndex = -1;
			float bestClosestCost = std::numeric_limits<float>::max();
			bool canInsert = false;
			for (int iRoute = 0; iRoute < routes.size(); iRoute++) {
				bool currCanInsert = routes[iRoute].canAddClient(m_allClients[currentCandidateId].demand);
				if (currCanInsert) {
					canInsert = true;
					float cost = getClosestInsertionCost(routes[iRoute], currentCandidateId);
					if (cost < bestClosestCost) {
						bestClosestCost = cost;
						closestRouteIndex = iRoute;
					}
				}
			}
			if (canInsert) {// Ignorar Rotas Fracionárias
				routes[closestRouteIndex].addClient(currentCandidateId, m_allClients[currentCandidateId].demand);
				candidatesList.remove(currentCandidateId);
			}
			else {
				candidateIndex++;
			}
		}
		if (candidatesList.size() > 0) {
			int randomVehicleType = Utils::getRandomInt(0, m_vehicleTypes.size());
			Vehicle randomVehicle = m_vehicleTypes[randomVehicleType];
			Route routeToAdd = m_routeCreator.createRoute(randomVehicle);
			routes.push_back(routeToAdd);
		}
	}
	for (int i = 0; i < routes.size(); i++) {
		if (!routes[i].clientsList.empty()) {
			result.push_back(routes[i]);
		}
	}
	return result;
}

float PRVFHEF::getClosestInsertionCost(const Route& route, int candidateId) const {
	//int cost = INT_MAX;
	float cost = 0;
	for (int iCand = 0; iCand < route.clientsList.size(); iCand++) {
		int currClientId = route.clientsList[iCand].id;
		float currCost = m_adjacencyCosts.getAdjacencyCosts(candidateId, currClientId);
		if (currCost < cost) {
			cost = currCost;
		}
		cost += currCost;
	}
	return cost;
}

std::vector<Route> PRVFHEF::splitReinsertion(const std::vector<Route>& solution, const Client& client, int forbiddenRouteId, bool& success) {
	std::vector<int> routesIndexWithResidual; // L
	std::vector<float> residualsList; // A
	std::vector<float> leastInsertionCosts; // U

	std::vector<Route> result = RouteDefs::copy(solution);

	int vehicles = solution.size();
	float totalResidual = 0;
	for (int i = 0; i < vehicles; i++) {
		const Route& r = solution[i];
		float currResidual = r.vehicle.capacity - r.getTotalDemand();
		if (r.id != forbiddenRouteId && currResidual > 0) {
			routesIndexWithResidual.push_back(i);
			residualsList.push_back(currResidual);
			totalResidual += currResidual;
		}
	}

	if (totalResidual > client.demand) { // Então existe o espaço para o cliente.
		for (int i = 0; i < routesIndexWithResidual.size(); i++) {
			std::vector<Client> clients;
			clients.push_back(client);
			float leastCostToInsert = RouteDefs::findBestInsertion(result[i], clients, m_adjacencyCosts).first;
			leastInsertionCosts.push_back(leastCostToInsert);
		}

		if (residualsList.size() != leastInsertionCosts.size()) {
			cout << "PRVFHEF::splitReinsertion , erro de tamanhos para knaapsack";
			exit(1);
		}

		return knaapSackGreedy(result, client, routesIndexWithResidual, residualsList, leastInsertionCosts, success);
	}
	return solution;
}

std::vector<Route> PRVFHEF::knaapSackGreedy(std::vector<Route>& solution, const Client& client,
	std::vector<int>& routesIndexWithResidual /*L*/, std::vector<float>& residualsList /*A*/, std::vector<float>& leastInsertionCosts /*U*/, bool& success) {
	std::vector<Route> result = RouteDefs::copy(solution);

	float currDemand = client.demand;

	struct Profit {
		int id;
		float value;
	};

	std::vector<Profit> profits(residualsList.size());

	for (int i = 0; i < residualsList.size(); i++) {
		float& residual = residualsList[i];
		float& cost = leastInsertionCosts[i];
		int profitval = cost / residual;
		Profit p;
		p.id = i;
		p.value = profitval;
		profits.push_back(p);
	}

	std::sort(profits.begin(), profits.end(), [](const Profit& p, const Profit& q) { return (float)p.value > (float)q.value; });
	int i = 0;
	while (currDemand > 0 && i < solution.size()) {
		Profit p = profits[i];
		int chosenRouteIndex = routesIndexWithResidual[p.id]; // The chosen route is the one that has residual space for insertion.
		float residual = residualsList[p.id];
		Route& chosenRoute = result[chosenRouteIndex];
		if (chosenRoute.canAddClient(client.demand)) {
			addClientToRoute(chosenRoute, client.id, residual);
			currDemand -= residual;
		}
		else {
			cout << "ERROR. PRVFHEF::knaapSackGreedy, Cannot add client to best profit route.";
		}
		i++;
	}
	if (i >= solution.size() && currDemand > 0) {
		cout << "WARN. PRVFHEF::knaapSackGreedy, Client still not fully added.";
	}
	return result;
}

std::vector<Route> PRVFHEF::emptyRoutes(const std::vector<Route>& solution) {
	std::vector<Route> resultSolution = solution;
	int tries = 0;
	while (solution.size() > m_maxVehicles && tries < solution.size()) {
		int mostEmptyRouteIndex = -1;
		int cargo = INT_MAX;
		for (int i = 0; i < solution.size(); i++) {
			int currCargo = m_auxiliaryStructures->cumulativeDelivery(i, solution[i].clientsList.size());
		}
		Route& chosen = resultSolution[mostEmptyRouteIndex];
		for (int i = 0; i < chosen.clientsList.size(); i++) {
			bool success = false;
			resultSolution = splitReinsertion(resultSolution, chosen.clientsList[i], mostEmptyRouteIndex, success);
			if (success) {
				chosen.removeClient(chosen.clientsList[i]);
				i--;
			}
		}
		tries++;
	}
	if (tries >= solution.size()) {
		cout << "emptyRoutes, TOO MANY TRIES";
		exit(1);
	}
}

std::vector<Route> PRVFHEF::reinsertSingleCustomer(std::vector<Route>& solution) {
	bool splitApplied = false;
	std::vector<Route> result = solution;
	do {
		splitApplied = false;

		std::vector<Route> stepSol = result;
		int evaluation = evaluate(solution);

		for (int i = 0; i < solution.size(); i++) {
			Route& current = solution[i];
			if (current.clientsList.size() == 1) {
				bool success = false;
				std::vector<Route> splitSol = splitReinsertion(result, current.clientsList[0], i, success);
				if (success) {
					int solEval = evaluate(splitSol);
					if (solEval < evaluation) {
						stepSol = splitSol;
						evaluation = solEval;
						current.removeClient(current.clientsList[0]);
						splitApplied = true;
					}
				}
			}
		}
		if (splitApplied) {
			result = stepSol;
		}
	} while (splitApplied);
	return result;
}

void PRVFHEF::vehicleRedimension(std::vector<Route>& routes, float currEval, float& resultEval) {
	float eval = currEval;
	for (Route& r : routes) {
		for (const Vehicle& v : m_vehicleTypes) {
			if (r.vehicle != v && v.capacity >= r.getTotalDemand()) {
				Vehicle oldVel = r.vehicle;
				r.vehicle = v;
				float ev = evaluate(routes);
				if (ev < eval) {
					eval = ev;
				}
				else {
					r.vehicle = oldVel;
				}
			}
		}
	}
	resultEval = eval;
}