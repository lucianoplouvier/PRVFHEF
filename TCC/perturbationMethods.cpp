#include "perturbationMethods.h"
#include "intrarouteStructures.h"
#include "fraction.h"

int MINSHIFTEXECS = 1;
int MAXSHIFTEXECS = 3;

int MINSWAPEXECS = 1;
int MAXSWAPEXECS = 3;

int MINRANDOMKSPLIT = 2;
int MAXRANDOMKSPLIT = 4;

std::vector<PERTURBATIONTYPES> perturbationMethods::getAll() {
	std::vector<PERTURBATIONTYPES> all;
	all.push_back(PERTURBATIONTYPES::MULTISWAP);
	all.push_back(PERTURBATIONTYPES::MULTISHIFT);
	all.push_back(PERTURBATIONTYPES::MERGE);
	all.push_back(PERTURBATIONTYPES::KSPLIT);
	all.push_back(PERTURBATIONTYPES::SPLIT);
	return all;
}

void removeFromCombination(std::vector<std::pair<int, int>>& vect, int index) {
	if (index < vect.size()) {
		std::vector<std::pair<int, int>>::iterator it = vect.begin();
		std::advance(it, index);
		if (it != vect.end()) {
			vect.erase(it);
		}
	}
}

// A ordem importa
std::vector<std::pair<int, int>> generatePossibleRouteCombinations(int routeASize, int routeBSize) {
	std::vector<std::pair<int, int>> result;
	if (routeASize > 0 && routeBSize > 0) {
		result.reserve(routeASize * (routeBSize - 1)); // num de combinacoes m�xima
		for (int i = 0; i < routeASize; i++) {
			for (int j = 0; j < routeBSize; j++) {
				if (i != j) {
					result.push_back(std::pair<int, int>(i, j));
				}
			}
		}
	}
	return result;
}

std::vector<std::pair<int, int>> generatePossibleSolutionCombinations(int solutionSize) {
	std::vector<std::pair<int, int>> result;
	if (solutionSize > 0) {
		result.reserve((solutionSize * (solutionSize - 1))); // num de combinacoes m�xima
		for (int i = 0; i < solutionSize; i++) {
			for (int j = 0; j < solutionSize; j++) {
				if (i != j) {
					result.push_back(std::pair<int, int>(i, j));
				}
			}
		}
	}
	return result;
}

std::pair<Route&, Route&> getRandomPair(std::vector<Route>& solution, std::vector<std::pair<int, int>>& possibleCombinations) {
	int qntCombinations = possibleCombinations.size();
	if (qntCombinations > 0) {
		int indexOfCombination = Utils::getRandomInt(0, qntCombinations - 1);
		std::pair<int, int> selectedCombination = possibleCombinations[indexOfCombination];
		Route& randomA = solution[selectedCombination.first];
		Route& randomB = solution[selectedCombination.second];
		// Remover combina��o selecionada
		removeFromCombination(possibleCombinations, indexOfCombination);
		return std::pair<Route&, Route&>(randomA, randomB);
	}
}

void checkDuplicated(Route& a) {
	std::vector<int> duplicated;
	for (int i = 0; i < a.clientsList.size(); i++) {
		int cId = a.clientsList[i].id;
		for (int j = 0; j < duplicated.size(); j++) {
			if (duplicated[j] == cId) {
				cout << "duplicationError\n";
			}
		}
		duplicated.push_back(cId);
	}
}

bool randomSwapping(Route& a, Route& b) {
	bool valid = false;
	int aSize = a.clientsList.size();
	int bSize = b.clientsList.size();
	int selectedA = -1, selectedB = -1;
	Client cA, cB;
	std::vector<std::pair<int, int>> combinationsList = generatePossibleRouteCombinations(a.clientsList.size(), b.clientsList.size());
	if (combinationsList.size() > 0) {
		do { // Executar s� movimentos v�lidos.
			int indexOfCombination = Utils::getRandomInt(0, combinationsList.size() - 1);
			std::pair<int, int> moves = combinationsList[indexOfCombination];
			removeFromCombination(combinationsList, indexOfCombination);
			selectedA = moves.first;
			selectedB = moves.second;
			Client rCA = a.clientsList[selectedA];
			Client rCB = b.clientsList[selectedB];
			double aTotalDemand = a.getTotalDemand();
			double bTotalDemand = b.getTotalDemand();
			double aDemand = rCA.demand;
			double bDemand = rCB.demand;
			if (((aTotalDemand - aDemand + bDemand) <= a.vehicle.capacity) && ((bTotalDemand - bDemand + aDemand) <= b.vehicle.capacity)) {
				valid = true;
				cA = rCA;
				cB = rCB;
			}
		} while (!valid && combinationsList.size() > 0);
		if (valid) {
			a.clientsList[selectedA] = cB;
			b.clientsList[selectedB] = cA;
		}
	}
	return valid;
}

// Retorna se conseguiu executar
bool randomShifting(Route& a, Route& b) {
	bool valid = false;
	int aSize = a.clientsList.size();
	int bSize = b.clientsList.size();
	int aIndex, bIndex;
	Client cA, cB;
	std::vector<std::pair<int, int>> combinationsList = generatePossibleRouteCombinations(a.clientsList.size(), b.clientsList.size());
	if (combinationsList.size() > 0) {
		do { // Executar s� movimentos v�lidos.
			int indexOfCombination = Utils::getRandomInt(0, combinationsList.size() - 1);
			std::pair<int, int> moves = combinationsList[indexOfCombination];
			removeFromCombination(combinationsList, indexOfCombination);

			int selectedA = moves.first;
			int selectedB = moves.second;
			Client rCA = a.clientsList[selectedA];
			Client rCB = b.clientsList[selectedB];
			double aTotalDemand = a.getTotalDemand();
			double bTotalDemand = b.getTotalDemand();
			double aDemand = rCA.demand;
			double bDemand = rCB.demand;
			if (((aTotalDemand - aDemand + bDemand) <= a.vehicle.capacity) && ((bTotalDemand - bDemand + aDemand) <= b.vehicle.capacity)) {
				valid = true;
				cA = rCA;
				cB = rCB;
				aIndex = selectedA;
				bIndex = selectedB;
			}
		} while (!valid && combinationsList.size() > 0);
		if (valid) {
			a.removeClient(cA);
			b.removeClient(cB);

			a.insertClient(cB, Utils::getRandomInt(0, a.clientsList.size()));
			b.insertClient(cA, Utils::getRandomInt(0, b.clientsList.size()));
		}
	}
	return valid;
}

void perturbationMethods::multipleSwap1_1(std::vector<Route>& solution) {
	int timesToExec = Utils::getRandomInt(MINSWAPEXECS, MAXSWAPEXECS);
	int solSize = solution.size();
	for (int i = 0; i < timesToExec; i++) {
		std::vector<std::pair<int, int>> possibilities = generatePossibleSolutionCombinations(solSize);
		bool success = false;
		while (!success && possibilities.size() > 0) {
			std::pair<Route&, Route&> randomRoutePair = getRandomPair(solution, possibilities);
			success = randomSwapping(randomRoutePair.first, randomRoutePair.second);
		}
		if (!success) break;
	}
}

void perturbationMethods::multiShift1_1(std::vector<Route>& solution) {
	int timesToExec = Utils::getRandomInt(MINSHIFTEXECS, MAXSHIFTEXECS);
	int solSize = solution.size();
	for (int i = 0; i < timesToExec; i++) {
		std::vector<std::pair<int, int>> possibilities = generatePossibleSolutionCombinations(solSize);
		bool success = false;
		while (!success && possibilities.size() > 0) {
			std::pair<Route&, Route&> randomRoutePair = getRandomPair(solution, possibilities);
			success = randomShifting(randomRoutePair.first, randomRoutePair.second);
		}
		if (!success) break;
	}
}

void perturbationMethods::ksplit(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels) {
	int timesToExec = Utils::getRandomInt(MINRANDOMKSPLIT, MAXRANDOMKSPLIT);
	std::vector<Client> randomClientsToChoose;
	for (const Client& c : clientList) {
		randomClientsToChoose.push_back(c);
	}
	int numOfClients = randomClientsToChoose.size();
	int maxExecutions = std::min(timesToExec, numOfClients); // N�o d� pra executar 7 vezes se n�o existirem 7 clientes.
	std::vector<Route> result = solution;
	for (int i = 0; i < maxExecutions; i++) {
		std::vector<Route> step = result;
		numOfClients = randomClientsToChoose.size();
		int randomClientIndex = Utils::getRandomInt(0, numOfClients - 1);
		int positionOfSingleRemoval = -1;
		auto it = randomClientsToChoose.begin();
		std::advance(it, randomClientIndex);
		RouteDefs::removeClientFromSolution(step, it->id, positionOfSingleRemoval);
		bool success = false;
		step = fractionRoute::splitReinsertion(step, RouteDefs::getOriginalClient(it->id, clientList), positionOfSingleRemoval, success, adjacencyCosts, vehiclesList);
		if (success) {
			result = step;
		}
		randomClientsToChoose.erase(it);
	}
	solution = result;
}

std::vector<Route> perturbationMethods::split(std::vector<Route> oldSolution, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehiclesList, RouteCreator& routeCreator, const std::vector<int>& availableVels, const std::vector<Client>& clientList) {
	std::vector<Route> solution = oldSolution;
	bool limitedVels = availableVels.size() > 0;
	std::vector<int> currentAvalVels; 
	if (limitedVels) {
		currentAvalVels = RouteDefs::calculateAvailableVels(solution, availableVels);
	}
	double capacitySpace = 0;
	for (int i = 0; i < currentAvalVels.size(); i++) {
		int& leftVelAmount = currentAvalVels[i];
		if (leftVelAmount> 0) {
			capacitySpace += vehiclesList[i].capacity;
		}
	}
	int routeIndex = 0;
	bool found = false;

	Vehicle smallestVehicle = RouteDefs::getSmallestVehicle(vehiclesList);
	Vehicle biggestVehicle = RouteDefs::getBiggestVehicle(vehiclesList);

	Route selected(0, Vehicle());

	std::vector<int> routeOptions; // Preenche lista rotas poss�veis
	for (int i = 0; i < solution.size(); i++) {
		if (solution[i].vehicle != smallestVehicle && solution[i].clientsList.size() > 0 && (!limitedVels || solution[i].getTotalDemand() <= capacitySpace)) {
			routeOptions.push_back(i);
		}
	}

	if (routeOptions.size()) {
		selected = (solution[Utils::getRandomInt(0, solution.size() - 1)]);
	}
	else {
		return oldSolution;
	}

	auto it = solution.begin();
	std::vector<Client> clients;
	for (Client c : solution[routeIndex].clientsList) {
		clients.push_back(c);
	}
	std::advance(it, routeIndex);
	solution.erase(it);
	const Vehicle* randomVehicle = NULL;
	int iClient = 0;
	bool success = true;
	while (iClient < clients.size() && success) {
		success = false;
		std::vector<Vehicle> possibleVels = vehiclesList;
		do {
			int randomVel = Utils::getRandomInt(0, possibleVels.size() - 1);
			if ((!limitedVels || currentAvalVels[randomVel] > 0) && vehiclesList[randomVel].capacity >= clients[iClient].demand && vehiclesList[randomVel].id != biggestVehicle.id) {
				randomVehicle = &vehiclesList[randomVel];
			}
			auto velIt = possibleVels.begin();
			std::advance(velIt, randomVel);
			possibleVels.erase(velIt);
			if (limitedVels) {
				currentAvalVels[randomVel]--;
			}
		} while (!randomVehicle && possibleVels.size() > 0);
		if (randomVehicle) { // TODO n�o entra aqui se n�o houver um ve�culo que caiba o cliente.
			Route newR = routeCreator.createRoute(*randomVehicle);
			while (iClient < clients.size() && clients[iClient].demand <= (newR.vehicle.capacity - newR.getTotalDemand())) {
				newR.addClient(clients[iClient]);
				iClient++;
			}
			if (newR.clientsList.size() > 0) {
				solution.push_back(newR);
			}
			success = true;
		}
		randomVehicle = NULL;
	}
	if (RouteDefs::isSolutionValid(solution, clientList, availableVels)) { // Caso n�o tenha conseguido alocar todas as rotas, n�o tinha espa�o suficiente.
		return solution;
	}
	else {
		return oldSolution;
	}
}

int findClosestRoute(std::vector<Route>& solution, Route* selected, AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehiclesList) {
	int closestRouteId = -1;
	double avgRouteDist = std::numeric_limits<double>::max();
	int indexResult = -1;
	Vehicle biggestVehicle = RouteDefs::getBiggestVehicle(vehiclesList);
	for (int i = 0; i < solution.size(); i++) {
		Route& r = solution[i];
		if (r.id != selected->id) {
			double totalDemand = r.getTotalDemand() + selected->getTotalDemand();
			if (totalDemand <= biggestVehicle.capacity) { // Sen�o n�o d� pra fazer o merge das rotas.
				int index = -1;
				double dist = 0;
				for (const Client& c : selected->clientsList) {
					for (const Client& cOther : r.clientsList) {
						dist += adjacencyCosts.getAdjacencyCosts(c.id, cOther.id);
					}
				}
				if (dist < avgRouteDist) {
					avgRouteDist = dist;
					indexResult = i;
				}
			}
		}
	}
	return indexResult;
}

void perturbationMethods::merge(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels) {
	bool limitedVels = availableVels.size() > 0;
	std::vector<int> currentAvalVels;
	if (limitedVels) {
		currentAvalVels = RouteDefs::calculateAvailableVels(solution, availableVels);
	}
	if (solution.size() > 2) {
		Vehicle biggestVehicle = RouteDefs::getBiggestVehicle(vehiclesList);
		int solutionSize = solution.size();
		Route* randomRoute = NULL;
		int randomRouteIndex = -1;
		std::vector<int> possibleRouteIndexes;
		possibleRouteIndexes.reserve(solutionSize);
		for (int i = 0; i < solutionSize; i++) {
			possibleRouteIndexes.push_back(i);
		}
		while (possibleRouteIndexes.size() > 0 && !randomRoute) {
			int rIndex = Utils::getRandomInt(0, possibleRouteIndexes.size() - 1);
			auto it = possibleRouteIndexes.begin();
			std::advance(it, rIndex);
			if (solution[rIndex].vehicle.id != biggestVehicle.id) {
				randomRoute = &solution[rIndex];
				randomRouteIndex = rIndex;
			}
			possibleRouteIndexes.erase(it);
		}
		if (randomRoute) {
			int otherRouteIndex = findClosestRoute(solution, randomRoute, adjacencyCosts, vehiclesList);
			if (otherRouteIndex > -1) {
				const Vehicle* randomVehicle = NULL;
				std::vector<int> possibleVehicles;
				possibleVehicles.reserve(vehiclesList.size());
				for (int i = 0; i < vehiclesList.size(); i++) {
					possibleVehicles.push_back(i);
				}
				while (!randomVehicle && possibleVehicles.size() > 0) {
					int randomVehicleIndex = Utils::getRandomInt(0, possibleVehicles.size() - 1);
					auto it = possibleVehicles.begin();
					std::advance(it, randomVehicleIndex);
					if ((!limitedVels || currentAvalVels[randomVehicleIndex] > 0) && randomRoute->getTotalDemand() + solution[otherRouteIndex].getTotalDemand() <= vehiclesList[randomVehicleIndex].capacity) {
						randomVehicle = &vehiclesList[randomVehicleIndex];
					}
					possibleVehicles.erase(it);
				}
				if (randomVehicle) {
					Route newRoute = creator.createRoute(*randomVehicle);
					for (int i = 0; i < randomRoute->clientsList.size(); i++) {
						Client clientToInsert(randomRoute->clientsList[i]);
						newRoute.addClient(clientToInsert.id, clientToInsert.demand);
					}
					Route& other = solution[otherRouteIndex];
					for (int i = 0; i < other.clientsList.size(); i++) {
						newRoute.addClientOrMerge(other.clientsList[i].id, other.clientsList[i].demand);
					}
					auto solIt = solution.begin();
					std::advance(solIt, randomRouteIndex);
					solution.erase(solIt);
					if (otherRouteIndex > randomRouteIndex) {
						otherRouteIndex--; // Se era uma rota mais pra frente, reduzir em 1.
					}
					solIt = solution.begin();
					std::advance(solIt, otherRouteIndex);
					solution.erase(solIt);
					solution.push_back(newRoute);
					auto intraroutes = intrarouteStructures::getAll();
					auto intraroutesIt = intraroutes.begin();
					solution = intrarouteStructures::executeAll(solution, adjacencyCosts, clientList);
				}
			}
		}
	}
}

void perturbationMethods::executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels) {
	switch (type)
	{
	case PERTURBATIONTYPES::MULTISWAP:
		multipleSwap1_1(solution);
		break;
	case PERTURBATIONTYPES::MULTISHIFT:
		multiShift1_1(solution);
		break;
	case PERTURBATIONTYPES::KSPLIT:
		ksplit(solution, adjacencyCosts, vehiclesList, clientList, availableVels);
		break;
	case PERTURBATIONTYPES::SPLIT:
		solution = split(solution, adjacencyCosts, vehiclesList, creator, availableVels, clientList);
		break;
	case PERTURBATIONTYPES::MERGE:
		merge(solution, adjacencyCosts, creator, vehiclesList, clientList, availableVels);
		break;
	default:
		break;
	}
}

void perturbationMethods::perturbate(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels) {
	std::vector<PERTURBATIONTYPES> pert = getAll();
	int selectedPerturbation = Utils::getRandomInt(0, pert.size() - 1);
	executePerturbation(solution, pert[selectedPerturbation], adjacencyCosts, creator, vehiclesList, clientList, availableVels);
}