#include "heterogeneousTransports.h"
#include "Utils.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int EXECUTIONTIMES = 450;
int MAXITERSNOIMPROVE = 3000;
int VAR_EXEC_TIMES = 5;

PRVFHEF::PRVFHEF(std::vector<float> clientsDemands, std::vector<ClientAdjacency> clientAdjacencies, std::vector<Vehicle> vehicleTypes, std::vector<float> depotTravelCost, std::string t, std::vector<int> availableVels, int vehicles) {
	m_t = t;
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
	int vItrs = vehicles != -1 ? vehicles : m_vehicleTypes.size();
	m_itrsToExecute = m_clientsCount + vItrs * VAR_EXEC_TIMES ;
	m_currIteration = 0;
	m_currIterationsWithoutImprove = 0;

	execute(vehicles, EXECUTIONTIMES, std::min(m_itrsToExecute, MAXITERSNOIMPROVE));
}

PRVFHEF::~PRVFHEF() {

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

	bool limitedVels = m_availableVels.size() > 0;

	std::vector<int> currVelList(m_availableVels);
	int candidatesLeft = candidates.size();
	for (auto iVel = 0; iVel < vehiclesCount && candidatesLeft > 0; iVel++) {
		int randomVelInt = Utils::getRandomInt(0, m_vehicleTypes.size() - 1);
		while (limitedVels && currVelList[randomVelInt] < 1) {
			bool hasOne = false;
			for (int a : currVelList) {
				if (a > 0) {
					hasOne = true;
					break;
				}
			}
			if (hasOne) {
				randomVelInt = Utils::getRandomInt(0, m_vehicleTypes.size() - 1);
			}
			else {
				randomVelInt = -1;
				break;
			}
		}
		if (randomVelInt > 0) {
			Route r = this->m_routeCreator.createRoute(m_vehicleTypes[randomVelInt]);
			if (limitedVels) currVelList[randomVelInt] -= 1;
			int candidateIndex = Utils::getRandomInt(0, candidates.size() - 1);
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
	}

	routes = paralelInsertion(routes, candidates, 0, m_availableVels);

	cout << "Initial Routes: " << routes.size() << '\n'; 

	// Adição de rotas vazias
	if (m_availableVels.size() == 0) {
		for (int i = 0; i < m_vehicleTypes.size(); i++) {
			Route emptyVelType = m_routeCreator.createRoute(m_vehicleTypes[i]);
			routes.push_back(emptyVelType);
		}
	}
	routes = fractionRoute::reinsertSingleCustomer(routes, m_adjacencyCosts);
	if (routes.size() > vehiclesCount) {
		routes = fractionRoute::emptyRoutes(routes, vehiclesCount, m_adjacencyCosts);
	}
	
	cout << "Comecando algoritmo \n";

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
			intrarouteList = intrarouteStructures::getAll();
		}
		else {
			intrarouteList.remove(selectedIntraroute);
		}
	}
	return result;
}

void PRVFHEF::checkAndRepair(std::vector<Route>& solution) {
	for (Route& r : solution) {
		for (int i = 0; i < r.clientsList.size(); i++) {
			Client c = r.clientsList[i];
			bool found = false;
			for (int j = i + 1; j < r.clientsList.size(); j++) {
				Client n = r.clientsList[j];
				if (n.id == c.id) {
					found = true;
					c.demand += n.demand;
					r.removeClient(n);
					j--;
				}
			}
			if (found) {
				r.removeClient(c.id);
				i--;
				std::vector<Client> cl;
				cl.push_back(c);
				r.insertClient(c, RouteDefs::findBestInsertion(r, cl, m_adjacencyCosts).second);
				RouteDefs::isSolutionValid(solution, m_allClients, m_availableVels);
			}
		}
	}
}

std::vector<Route> PRVFHEF::rvnd(std::vector<Route>& currSol, float evaluation) {
	// Cria lista de vizinhança
	//m_auxiliaryStructures->recalculate(currSol);
	std::list<INTERROUTETYPES> neighborhoodList = interrouteStructures::getAll();
	std::vector<Route> finalResult = currSol;
	float finalEval = evaluation;
	// Atualizar estruturas de dados auxiliares
	while (!neighborhoodList.empty()) {
		// Escolhe aleatóriamente uma vizinhança.
		std::list<INTERROUTETYPES>::iterator iterator = neighborhoodList.begin();

		int selectedInterrouteIndex = Utils::getRandomInt(0, neighborhoodList.size() - 1);
		std::advance(iterator, selectedInterrouteIndex);
		INTERROUTETYPES selectedInterroute = *iterator;

		// Execução da interrota.
		std::vector<Route> interrouteResult = interrouteStructures::executeInterroute(selectedInterroute, finalResult, finalEval, m_auxiliaryStructures, m_adjacencyCosts, m_allClients, m_availableVels);
		float roundEval = RouteDefs::evaluate(interrouteResult, m_adjacencyCosts);
		if (roundEval < finalEval) {
			checkAndRepair(interrouteResult);
			finalResult = intraroute(interrouteResult, roundEval);
			finalEval = RouteDefs::evaluate(finalResult, m_adjacencyCosts);
			neighborhoodList = interrouteStructures::getAll();
			// Reiniciar a lista de vizinhanças.
			//m_auxiliaryStructures->recalculate(currSol);
		}
		else {
			// Remoção da vizinhança.
			neighborhoodList.remove(selectedInterroute);
		}
	}
	return finalResult;
}

void PRVFHEF::execute(int initialVehicles, int iterations, int maxItersNoImprove) {
	cout << "Executando " << iterations << " Iteracoes com maxIter = " << maxItersNoImprove << "." << "\n";
	time_t start, end;
	time(&start);
	
	int vehicles = initialVehicles;

	if (vehicles == -1) {
		vehicles = m_vehicleTypes.size();
	}
	std::vector<Route> finalSolution;
	float finalEval = std::numeric_limits<float>::max();
	float oldEval;
	for (int algItrs = 1; algItrs <= iterations; algItrs++) {
		m_currIteration = algItrs;
		// Executa cada rodada do algoritmo, onde uma nova solução é procurada a partir do 0.
		m_auxiliaryStructures = new AuxiliaryStructures(&m_adjacencyCosts, interrouteStructures::getAll().size());
		m_routeCreator.reset();
		std::vector<Route> solution = createInitialSolution(vehicles);
		oldEval = RouteDefs::evaluate(solution, m_adjacencyCosts);
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
		std::ostringstream ss;
		ss << finalEval;
		std::string s(ss.str());
		cout << "Fim It: " << algItrs << ", Avaliacao da rodada: " << oldEval << ".\nMelhor Eval Ate agora:" << s << " \n";
		delete m_auxiliaryStructures;
		m_auxiliaryStructures = NULL;
	}
	float endEval = RouteDefs::evaluate(finalSolution, m_adjacencyCosts);
	cout << "Solucao valida: " << RouteDefs::isSolutionValid(finalSolution, m_allClients, m_availableVels) << "\n";
	time(&end);
	double executionTime = double(end - start);
	ofstream exitStream;
	exitStream.open("saida Taillard_" + m_t + ".txt");
	exitStream << "Tempo de execucao: " << executionTime << " segundos." << "\n";
	exitStream << "\n";
	if (!verifySolutionValid(finalSolution)) {
		printSolution(finalEval, finalSolution, exitStream);
	}
	else {
		exitStream << "Erro no final!\n";
	}
}

bool PRVFHEF::verifySolutionValid(const std::vector<Route>& solution) {
	std::vector<float> clientDemands;
	bool error = false;
	for (int i = 0; i < m_allClients.size(); i++) {
		clientDemands.push_back(m_allClients[i].demand);
	}

	for (int i = 0; i < solution.size(); i++) {
		float velCap = solution[i].vehicle.capacity;
		float totalDemand = solution[i].getTotalDemand();
		float rest = velCap - totalDemand;
		if (rest < 0) {
			cout << "ERROR. Usando mais espaco do veiculo que aceitavel. " << velCap << " - " << totalDemand << " = " << rest << "\n";
			error = true;
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
			error = true;
		}
		else if (clientDemands[i] < 0) {
			cout << "ERROR. Cliente de id" << i << " com demanda negativa. Valor: " << clientDemands[i];
			error = true;
		}
	}
	bool solutionValid = RouteDefs::isSolutionValid(solution, m_allClients, m_availableVels);
	if (!solutionValid) {
		error = true;
		cout << "PRVFHEF::verifySolutionValid, Solucao nao valida.";
	}
	return error;
}

void PRVFHEF::printSolution(float eval, const std::vector<Route>& solution, ofstream& stream) {
	int totalMelhoriasInterroute = interrouteStructures::getInterrouteSumImprove(); 
	int totalMelhoriasIntrarroute = intrarouteStructures::getIntrarrouteSumImprove();
	float evalEnd = RouteDefs::evaluate(solution, m_adjacencyCosts);
	std::ostringstream ss;
	ss << evalEnd;
	std::string s(ss.str());
	stream << "Resultado iteracao. EVAL: " << s << "\n";
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
		stream << "WARN: Não houve melhorias interroute!\n";
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
		stream << "WARN: Não houve melhorias intraroute!\n";
	}
	stream << "\n";
	stream << "Rota: \n";
	for (const Route& r : solution) {
		if (r.clientsList.size() > 0) {
			float velCap = r.vehicle.capacity;
			float totalDemand = r.getTotalDemand();
			float rest = velCap - totalDemand;
			if (rest < 0) {
				stream << "ERROR. Usando mais espaco do veiculo que aceitavel. " << velCap << " - " << totalDemand << " = " << rest << "\n";
			}
			stream << "Veiculo: " << r.vehicle.id << " Capacidade: " << velCap << " Custo: " << r.vehicle.cost << " Custo V: " << r.vehicle.travelCost << "\n";
			stream << "Clientes:\n";
			for (int j = 0; j < r.clientsList.size(); j++) {
				stream << "(I:" << r.clientsList[j].id + 1 << ", D:" << r.clientsList[j].demand << ")";
				if (j + 1 < r.clientsList.size()) {
					stream << ", ";
				}
			}
			stream << "\nSobrou do veiculo: " << rest << "\n";
			stream << "Custo Viagem Rota: " << RouteDefs::evaluateRoute(r, m_adjacencyCosts) << "\n";
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
	//bool criteria = Utils::getRandomInt(0, 1); // 0 = CIMPV, 1 = CIMBVM
	bool criteria = 1;
	float gamma = 0;
	if (criteria == 1) {
		std::vector<float> gammaCosts;
		float factor = 0;
		while (factor <= 1.75f) {
			gammaCosts.push_back(factor);
			factor += 0.05f;
		}
		gamma = gammaCosts[Utils::getRandomInt(0, gammaCosts.size() - 1)];
	}

	std::vector<Route> result;
	while (candidatesList.size() > 0) {
		while (candidatesList.size() > 0 && canAddAnyToARoute(routes, candidatesList)) {
			auto itCandidate = candidatesList.begin();
			int globalRouteIndex = 0;
			int bestInsertPosition = 0;
			int bestCandidate = 0;
			int currCandidate = 0;
			float globalBestCost = std::numeric_limits<float>::max();
			bool canInsert = false;
			while (itCandidate != candidatesList.end()) {
				int currentCandidateId = *itCandidate;
				int closestRouteIndex = -1;
				int insertPosition = 0;
				float bestClosestCost = std::numeric_limits<float>::max();
				for (int iRoute = 0; iRoute < routes.size(); iRoute++) {
					bool currCanInsert = routes[iRoute].canAddClient(m_allClients[currentCandidateId].demand);
					if (currCanInsert) {
						canInsert = true;
						int currPos = 0;
						float currCost = std::numeric_limits<float>::max();
						if (criteria == 0) { // CIMPV
							currCost = getClosestInsertionCost(routes[iRoute], currPos, currentCandidateId);
						}
						else { // CIMBVM
							currCost = getCheapestInsertionCost(routes[iRoute], currPos, currentCandidateId, gamma);
						}
						if (currCost < bestClosestCost) {
							bestClosestCost = currCost;
							insertPosition = currPos;
							closestRouteIndex = iRoute;
						}
					}
				}
				if (bestClosestCost < globalBestCost) {
					globalBestCost = bestClosestCost;
					bestInsertPosition = bestInsertPosition;
					globalRouteIndex = closestRouteIndex;
					bestCandidate = currentCandidateId;
				}
				itCandidate++;
			}
			if (canInsert) {// Ignorar Rotas Fracionárias
				Client c(m_allClients[bestCandidate]);
				routes[globalRouteIndex].insertClient(c, bestInsertPosition);
				candidatesList.remove(bestCandidate);
			}
			else {
				cout << "ERROR. Não conseguiu inserir mas em teoria podia.\n";
				exit(1);
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
				else { // Insere um veículo ficticio
					Vehicle ficticious = getBiggestVehicle(m_vehicleTypes);
					ficticious.capacity *= numeric_limits<float>::max();
					ficticious.cost *= 10000;
					ficticious.id = m_vehicleTypes.size();
					ficticious.travelCost *= 10000;
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

float PRVFHEF::getClosestInsertionCost(const Route& route, int& position, int candidateId) const {
	int routeSize = route.clientsList.size();
	position = routeSize;
	float cost = m_adjacencyCosts.depotTravel[candidateId];
	if (routeSize > 0) {
		cost += m_adjacencyCosts.depotTravel[route.clientsList[0].id];
		for (int i = 1; i < route.clientsList.size(); i++) {
			cost += m_adjacencyCosts.getAdjacencyCosts(i, i-1);
		}
	}
	else {
		cost *= 2;
	}
	return cost;
}

float PRVFHEF::getCheapestInsertionCost(const Route& route, int& position, int candidateId, float y) const {
	float travelDepot = m_adjacencyCosts.depotTravel[candidateId];
	float gamma = y * (2 * travelDepot);
	position = 0;
	float cost = std::numeric_limits<float>::max();
	int clientsInRoute = route.clientsList.size();
	cost = travelDepot - gamma;
	if (clientsInRoute > 0) {
		float clientdemand = m_allClients[candidateId].demand;
		for (int i = 1; i <= clientsInRoute; i++) {
			int prevId = route.clientsList[i - 1].id;
			float currCost;
			if (i == clientsInRoute) {
				currCost = m_adjacencyCosts.getAdjacencyCosts(prevId, candidateId) + travelDepot - gamma;
			}
			else {
				int currId = route.clientsList[i].id;
				currCost = m_adjacencyCosts.getAdjacencyCosts(prevId, candidateId) + m_adjacencyCosts.getAdjacencyCosts(candidateId, currId) - m_adjacencyCosts.getAdjacencyCosts(prevId, currId) - gamma;
			}
			if (currCost < cost) {
				cost = currCost;
				position = i;
			}
		}
	}
	else {
		cost += travelDepot;
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
							if (limitedVels) {
								currAvalVels[i] = currAvalVels[i] - 1;
								for (int j = 0; j < m_vehicleTypes.size(); j++) {
									Vehicle vel = m_vehicleTypes[j];
									if (v.id == vel.id) { // Repor o veículo.
										currAvalVels[j] += 1; 
										break;
									}
								}
							}
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