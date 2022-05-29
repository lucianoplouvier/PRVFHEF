#include "heterogeneousTransports.h"
#include "Utils.h"

using namespace std;

int EXECUTIONTIMES = 1;
int MAXITERSNOIMPROVE = 200;

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
	execute(vehicles, EXECUTIONTIMES, MAXITERSNOIMPROVE);
}

PRVFHEF::~PRVFHEF() {

}

int PRVFHEF::estimateVehicles(const std::vector<Client>& allClients) const {
	Vehicle biggestVehicle = RouteDefs::getBiggestVehicle(m_vehicleTypes);

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
		std::vector<Route> roundResult = intrarouteStructures::execute(selectedIntraroute, result, m_adjacencyCosts);
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
	for (int algItrs = 0; algItrs < iterations; algItrs++) {
		// Executa cada rodada do algoritmo, onde uma nova solução é procurada a partir do 0.
		m_auxiliaryStructures = new AuxiliaryStructures(&m_adjacencyCosts, interrouteStructures::getAll().size());
		m_routeCreator.reset();
		std::vector<Route> solution = createInitialSolution(vehicles);
		float oldEval = evaluate(solution);
		std::vector<Route> solutionOptimized = rvnd(solution, oldEval);
		oldEval = evaluate(solutionOptimized);
		finalSolution = solutionOptimized;
		for (int ilsIters = 0; ilsIters < maxItersNoImprove; ilsIters++) {
			perturbationMethods::perturbate(solutionOptimized, m_adjacencyCosts, m_routeCreator, m_vehicleTypes);
			verifySolutionValid(solutionOptimized);
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
	for (Route& r : finalSolution) {
		for (Client& c : r.clientsList) {
			cout << c.id << ",";
		}
		cout << "\n";
	}
	cout << "\n";
	cout << "\n";
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

void PRVFHEF::vehicleRedimension(std::vector<Route>& routes, float currEval, float& resultEval) {
	float eval = currEval;
	for (Route& r : routes) {
		if (r.clientsList.size() > 0) {
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
	}
	resultEval = eval;
}