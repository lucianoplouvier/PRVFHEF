#include "heterogeneousTransports.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int EXECUTIONTIMES = 10;
int MAXITERSNOIMPROVE = 5000;

PRVFHEF::PRVFHEF(std::vector<float> clientsDemands, std::vector<ClientAdjacency> clientAdjacencies, std::vector<Vehicle> vehicleTypes, std::vector<float> depotTravelCost, std::vector<int> availableVels, int vehicles) {
	m_clientsCount = clientsDemands.size();
	m_allClients.resize(m_clientsCount);
	m_vehicleTypes = vehicleTypes;
	m_availableVels = availableVels;
	createAdjacencyMatrix(m_clientsCount, clientAdjacencies, depotTravelCost);
	for (auto i = 0; i < clientsDemands.size(); i++) {
		Client c;
		c.id = i;
		c.demand = clientsDemands[i];
		m_allClients[i] = c;
	}
	m_clientsOriginalDemands = clientsDemands;
	m_auxiliaryStructures = NULL;
	//int itrsToExecute = vehicles != -1 ? vehicles * m_clientsCount : estimateVehicles(m_allClients) * m_clientsCount * 10;
	int itrsToExecute = 5000;
	m_currIteration = 0;
	m_currIterationsWithoutImprove = 0;

	execute(vehicles, EXECUTIONTIMES, std::min(itrsToExecute, MAXITERSNOIMPROVE));
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
		int candidateIndex = Utils::getRandomInt(0, candidates.size() - 1);
		int candidateId;
		auto iterator = candidates.begin();
		std::advance(iterator, candidateIndex);
		candidateId = *iterator;
		bool added = r.addClient(candidateId, m_allClients[candidateId].demand); // Ignorar Rotas Fracion�rias
		if (added) {
			candidates.remove(candidateId);
			candidatesLeft -= 1;
			routes.push_back(r);
		}
	}

	routes = paralelInsertion(routes, candidates, 0, m_availableVels);

	cout << "Initial Routes: " << routes.size() << '\n'; 

	// Adi��o de rotas vazias
	if (m_availableVels.size() == 0) {
		for (int i = 0; i < m_vehicleTypes.size(); i++) {
			Route emptyVelType = m_routeCreator.createRoute(m_vehicleTypes[i]);
			routes.push_back(emptyVelType);
		}
	}
	routes = fractionRoute::reinsertSingleCustomer(routes, m_adjacencyCosts, m_allClients);
	if (routes.size() > vehiclesCount) {
		routes = fractionRoute::emptyRoutes(routes, vehiclesCount, m_adjacencyCosts, m_allClients);
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
	std::vector<Route> result = solution;
	float bestEval = evaluation;
	while (!intrarouteList.empty()) {
		std::list<INTRAROUTETYPES>::iterator iterator = intrarouteList.begin();
		int selectedIntrarouteIndex = Utils::getRandomInt(0, intrarouteList.size() - 1);
		std::advance(iterator, selectedIntrarouteIndex);
		INTRAROUTETYPES selectedIntraroute = *iterator;
		std::vector<Route> roundResult = intrarouteStructures::execute(selectedIntraroute, result, m_adjacencyCosts, m_allClients);
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
	// Cria lista de vizinhan�a
	m_auxiliaryStructures->recalculate(currSol);
	std::list<INTERROUTETYPES> neighborhoodList = interrouteStructures::getAll();
	std::vector<Route> finalResult = currSol;
	float finalEval = RouteDefs::evaluate(finalResult, m_adjacencyCosts);
	// Atualizar estruturas de dados auxiliares
	while (!neighborhoodList.empty()) {
		// Escolhe aleat�riamente uma vizinhan�a.
		std::list<INTERROUTETYPES>::iterator iterator = neighborhoodList.begin();

		int selectedInterrouteIndex = Utils::getRandomInt(0, neighborhoodList.size() - 1);
		std::advance(iterator, selectedInterrouteIndex);
		INTERROUTETYPES selectedInterroute = *iterator;

		// Execu��o da interrota.
		std::vector<Route> roundResult = finalResult;
		std::vector<Route> interrouteResult = interrouteStructures::executeInterroute(selectedInterroute, roundResult, evaluation, m_auxiliaryStructures, m_adjacencyCosts, m_allClients, m_availableVels);
		float roundEval = RouteDefs::evaluate(interrouteResult, m_adjacencyCosts);
		if (roundEval < finalEval) {
			finalResult = intraroute(interrouteResult, roundEval);
			finalEval = RouteDefs::evaluate(finalResult, m_adjacencyCosts);
			neighborhoodList = interrouteStructures::getAll();
			// Reiniciar a lista de vizinhan�as.
		}
		else {
			// Remo��o da vizinhan�a.
			neighborhoodList.remove(selectedInterroute);
		}
		m_auxiliaryStructures->recalculate(currSol);
	}
	return finalResult;
}

void PRVFHEF::execute(int initialVehicles, int iterations, int maxItersNoImprove) {
	cout << "Executando " << iterations << " Itera��es com maxIter = " << maxItersNoImprove << "." << "\n";
	time_t start, end;
	time(&start);
	/*
	int vehicles = initialVehicles;
	if (vehicles == -1) {
		vehicles = estimateVehicles(m_allClients);
	}
	*/
	int vehicles = m_vehicleTypes.size();
	std::vector<Route> finalSolution;
	float finalEval = std::numeric_limits<float>::max();
	for (int algItrs = 1; algItrs <= iterations; algItrs++) {
		m_currIteration = algItrs;
		// Executa cada rodada do algoritmo, onde uma nova solu��o � procurada a partir do 0.
		m_auxiliaryStructures = new AuxiliaryStructures(&m_adjacencyCosts, interrouteStructures::getAll().size());
		m_routeCreator.reset();
		std::vector<Route> solution = createInitialSolution(vehicles);
		float oldEval = RouteDefs::evaluate(solution, m_adjacencyCosts);
		solution = rvnd(solution, oldEval);
		oldEval = RouteDefs::evaluate(solution, m_adjacencyCosts);
		if (oldEval < finalEval) {
			finalEval = oldEval;
			finalSolution = solution;
		}
		for (int ilsIters = 0; ilsIters < maxItersNoImprove; ilsIters++) {
			std::vector<Route> solutionOptimized(solution);
			m_currIterationsWithoutImprove = ilsIters;
			perturbationMethods::perturbate(solutionOptimized, m_adjacencyCosts, m_routeCreator, m_vehicleTypes, m_allClients, m_availableVels);
			m_auxiliaryStructures->recalculate(solutionOptimized);
			solutionOptimized = rvnd(solutionOptimized, oldEval);
			float evaluation = RouteDefs::evaluate(solutionOptimized, m_adjacencyCosts);
			if (evaluation < oldEval) {
				oldEval = evaluation;
				solution = solutionOptimized;
				ilsIters = -1;
			}
		}
		if (oldEval < finalEval) {
			finalEval = oldEval;
			finalSolution = solution;
			finalEval = vehicleRedimension(finalSolution, finalEval, m_availableVels);
		}
		cout << "Fim It: " << algItrs << ". Melhor Eval Ate agora:" << finalEval << " \n";
		delete m_auxiliaryStructures;
		m_auxiliaryStructures = NULL;
	}
	time(&end);
	double executionTime = double(end - start);
	ofstream exitStream;
	exitStream.open("saida.txt");
	exitStream << "Tempo de execu��o: " << executionTime << " segundos." << "\n";
	exitStream << "\n";
	if (verifySolutionValid(finalSolution)) {
		printSolution(finalEval, finalSolution, exitStream);
	}
	else {
		exitStream << "Erro no final!\n";
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
	bool solutionValid = RouteDefs::isSolutionValid(solution, m_allClients, m_availableVels);
	if (!solutionValid) {
		error = true;
		cout << "PRVFHEF::verifySolutionValid, Solu��o n�o v�lida.";
	}
	return error;
}

void PRVFHEF::printSolution(float eval, const std::vector<Route>& solution, ofstream& stream) {
	int totalMelhoriasInterroute = interrouteStructures::getInterrouteSumImprove(); 
	int totalMelhoriasIntrarroute = intrarouteStructures::getIntrarrouteSumImprove();
	stream << "Resultado iteracao. EVAL: " << RouteDefs::evaluate(solution, m_adjacencyCosts) << "\n";
	stream << "\n";
	if (totalMelhoriasInterroute > 0) {
		stream << "Melhorias Interrotas:" << "\n";
		stream << "shift10Improve: " << interrouteStructures::getShift10Improve() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "shift20Improve: " << interrouteStructures::getShift20Improve() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "swap11Improve: " << interrouteStructures::getSwap11Improve() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "swap21Improve: " << interrouteStructures::getSwap21Improve() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "swap11SImprove: " << interrouteStructures::getSwap11SImprove() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "swap21SImprove: " << interrouteStructures::getSwap21SImprove() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "crossImprove: " << interrouteStructures::getCrossImprove() * 100 / totalMelhoriasInterroute << "%" << "\n";
		stream << "kSplitImprove: " << interrouteStructures::getKSplitImprove() * 100 / totalMelhoriasInterroute << "%" << "\n";
	}
	else {
		stream << "WARN: N�o houve melhorias interroute!\n";
	}
	stream << "\n";
	if (totalMelhoriasIntrarroute > 0) {
		stream << "Melhorias Intrarrotas:" << "\n";
		stream << "shiftImprove: " << intrarouteStructures::getShiftImprove() * 100 / totalMelhoriasIntrarroute << "%" << "\n";
		stream << "swapImprove: " << intrarouteStructures::getSwapImprove() * 100 / totalMelhoriasIntrarroute << "%" << "\n";
		stream << "orOpt2Improve: " << intrarouteStructures::getorOpt2Improve() * 100 / totalMelhoriasIntrarroute << "%" << "\n";
		stream << "orOpt3Improve: " << intrarouteStructures::getorOpt3Improve() * 100 / totalMelhoriasIntrarroute << "%" << "\n";
		stream << "twoOptImprove: " << intrarouteStructures::getTwoOptImprove() * 100 / totalMelhoriasIntrarroute << "%" << "\n";
	}
	else {
		stream << "WARN: N�o houve melhorias intraroute!\n";
	}
	stream << "\n";
	stream << "Rota: \n";
	for (const Route& r : solution) {
		if (r.clientsList.size() > 0) {
			float velCap = r.vehicle.capacity;
			float totalDemand = r.getTotalDemand();
			float rest = velCap - totalDemand;
			if (rest < 0) {
				stream << "ERROR. Usando mais espa�o do veiculo que aceitavel. " << velCap << " - " << totalDemand << " = " << rest << "\n";
			}
			else {
				stream << "Veiculo: " << r.vehicle.id << " Capacidade: " << velCap << " Custo: " << r.vehicle.cost << " Custo V: " << r.vehicle.travelCost << "\n";
				stream << "Clientes:\n";
				for (int j = 0; j < r.clientsList.size(); j++) {
					stream << "(I:" << r.clientsList[j].id << ", D:" << r.clientsList[j].demand << ")";
					if (j + 1 < r.clientsList.size()) {
						stream << ", ";
					}
				}
				stream << "\nSobrou do veiculo: " << rest << "\n";
				stream << "Custo Viagem Rota: " << RouteDefs::evaluateRoute(r, m_adjacencyCosts) << "\n";
			}
			stream << "\n";
		}
	}
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

std::vector<Route> PRVFHEF::paralelInsertion(std::vector<Route>& routes, std::list<int>& candidatesList, bool insertionCriteria, std::vector<int> availableVels) {
	std::vector<int> avalVels; 
	if (availableVels.size() > 0) {
		avalVels = RouteDefs::calculateAvailableVels(routes, availableVels);
	}
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
			if (canInsert) {// Ignorar Rotas Fracion�rias
				routes[closestRouteIndex].addClient(currentCandidateId, m_allClients[currentCandidateId].demand);
				candidatesList.remove(currentCandidateId);
			}
			else {
				candidateIndex++;
			}
		}
		if (candidatesList.size() > 0) {
			if (availableVels.size() == 0) {
				int randomVehicleType = Utils::getRandomInt(0, m_vehicleTypes.size() - 1);
				Vehicle randomVehicle = m_vehicleTypes[randomVehicleType];
				Route routeToAdd = m_routeCreator.createRoute(randomVehicle);
				routes.push_back(routeToAdd);
			}
			else {
				std::vector<int> currentAvalVels;
				for (int i = 0; i < avalVels.size(); i++) {
					if (avalVels[i] > 0) {
						currentAvalVels.push_back(i);
					}
				}
				if (currentAvalVels.size() > 0) {
					int randomVehicleType = Utils::getRandomInt(0, currentAvalVels.size() - 1);
					Vehicle randomVehicle = m_vehicleTypes[currentAvalVels[randomVehicleType]];
					Route routeToAdd = m_routeCreator.createRoute(randomVehicle);
					routes.push_back(routeToAdd);
					avalVels[currentAvalVels[randomVehicleType]] = avalVels[currentAvalVels[randomVehicleType]] - 1;
				}
				else { // Insere um ve�culo ficticio
					Vehicle ficticious;
					ficticious.capacity = numeric_limits<float>::max();
					ficticious.cost = numeric_limits<float>::max() / 2;
					ficticious.id = m_vehicleTypes.size();
					ficticious.travelCost = 1;
					ficticious.ficticous = true;
					Route routeToAdd = m_routeCreator.createRoute(ficticious);
					routes.push_back(routeToAdd);
					m_vehicleTypes.push_back(ficticious);
					m_availableVels.push_back(0);
				}
			}
		}
	}
	for (int i = 0; i < routes.size(); i++) {
		//if (!routes[i].clientsList.empty()) {
			result.push_back(routes[i]);
		//}
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

float PRVFHEF::vehicleRedimension(std::vector<Route>& routes, float currEval, const std::vector<int>& availableVels) {
	std::vector<int> currAvalVels;
	bool limitedVels = availableVels.size() > 0;
	if (availableVels.size() > 0) {
		currAvalVels = RouteDefs::calculateAvailableVels(routes, availableVels);
	}
	float eval = currEval;
	for (Route& r : routes) {
		if (r.clientsList.size() > 0) {
			for (int i = 0; i < m_vehicleTypes.size(); i++) {
				if (!limitedVels || currAvalVels[i] > 0) {
					const Vehicle& v = m_vehicleTypes[i];
					if (r.vehicle != v && v.capacity >= r.getTotalDemand()) {
						Vehicle oldVel = r.vehicle;
						r.vehicle = v;
						float ev = RouteDefs::evaluate(routes, m_adjacencyCosts);
						if (ev < eval) {
							eval = ev;
							currAvalVels[i] = currAvalVels[i] - 1;
						}
						else {
							r.vehicle = oldVel;
						}
					}
				}
			}
		}
	}
	return eval;
}