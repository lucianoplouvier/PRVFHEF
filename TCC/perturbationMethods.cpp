#include "perturbationMethods.h"
#include "intrarouteStructures.h"

int MINSHIFTEXECS = 1;
int MAXSHIFTEXECS = 2;

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
		result.reserve(routeASize * (routeBSize - 1)); // num de combinacoes máxima
		for (int i = 0; i < routeASize; i++) {
			for (int j = 0; j < routeBSize; j++) {
				result.push_back(std::pair<int, int>(i, j));
			}
		}
	}
	return result;
}

std::vector<std::pair<int, int>> generatePossibleSolutionCombinations(int solutionSize) {
	std::vector<std::pair<int, int>> result;
	if (solutionSize > 0) {
		result.reserve((solutionSize * (solutionSize - 1))); // num de combinacoes máxima
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
		// Remover combinação selecionada
		removeFromCombination(possibleCombinations, indexOfCombination);
		return std::pair<Route&, Route&>(randomA, randomB);
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
		do { // Executar só movimentos válidos.
			int indexOfCombination = Utils::getRandomInt(0, combinationsList.size());
			std::pair<int, int> moves = combinationsList[indexOfCombination];
			removeFromCombination(combinationsList, indexOfCombination);
			selectedA = moves.first;
			selectedB = moves.second;
			Client rCA = a.clientsList[selectedA];
			Client rCB = b.clientsList[selectedB];
			float aTotalDemand = a.getTotalDemand();
			float bTotalDemand = b.getTotalDemand();
			float aDemand = rCA.demand;
			float bDemand = rCB.demand;
			if (((aTotalDemand - aDemand + bDemand) < a.vehicle.capacity) && ((bTotalDemand - bDemand + aDemand) < b.vehicle.capacity)) {
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
	Client cA, cB;
	std::vector<std::pair<int, int>> combinationsList = generatePossibleRouteCombinations(a.clientsList.size(), b.clientsList.size());
	if (combinationsList.size() > 0) {
		do { // Executar só movimentos válidos.
			int indexOfCombination = Utils::getRandomInt(0, combinationsList.size());
			std::pair<int, int> moves = combinationsList[indexOfCombination];
			removeFromCombination(combinationsList, indexOfCombination);

			int selectedA = moves.first;
			int selectedB = moves.second;
			Client rCA = a.clientsList[selectedA];
			Client rCB = b.clientsList[selectedB];
			float aTotalDemand = a.getTotalDemand();
			float bTotalDemand = b.getTotalDemand();
			float aDemand = rCA.demand;
			float bDemand = rCB.demand;
			if (((aTotalDemand - aDemand + bDemand) < a.vehicle.capacity) && ((bTotalDemand - bDemand + aDemand) < b.vehicle.capacity)) {
				valid = true;
				cA = rCA;
				cB = rCB;
			}
		} while (!valid && combinationsList.size() > 0);
		if (valid) {
			a.removeClient(cA);
			b.removeClient(cB);
			aSize--;
			bSize--;
			a.insertClient(cB, Utils::getRandomInt(0, aSize));
			b.insertClient(cA, Utils::getRandomInt(0, bSize));
		}
	}
	return valid;
}

void perturbationMethods::multipleSwap1_1(std::vector<Route>& solution) {
	int timesToExec = Utils::getRandomInt(MINSHIFTEXECS, MAXSHIFTEXECS);
	int solSize = solution.size();
	for (int i = 0; i < timesToExec; i++) {
		std::vector<std::pair<int, int>> possibilities = generatePossibleSolutionCombinations(solSize);
		std::pair<Route&, Route&> randomRoutePair = getRandomPair(solution, possibilities);
		randomSwapping(randomRoutePair.first, randomRoutePair.second);
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
	}
}

void perturbationMethods::split(std::vector<Route>& solution) {
}

void perturbationMethods::merge(std::vector<Route>& solution) {
}

std::vector<PERTURBATIONTYPES> perturbationMethods::getAll() {
	std::vector<PERTURBATIONTYPES> all;
	all.push_back(PERTURBATIONTYPES::MULTISWAP);
	all.push_back(PERTURBATIONTYPES::MULTISHIFT);
	all.push_back(PERTURBATIONTYPES::SPLIT);
	all.push_back(PERTURBATIONTYPES::MERGE);
	return all;
}

void perturbationMethods::executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type) {
	switch (type)
	{
	case PERTURBATIONTYPES::MULTISWAP:
		multipleSwap1_1(solution);
		break;
	case PERTURBATIONTYPES::MULTISHIFT:
		multiShift1_1(solution);
		break;
	case PERTURBATIONTYPES::SPLIT:
		split(solution);
		break;
	case PERTURBATIONTYPES::MERGE:
		merge(solution);
		break;
	default:
		break;
	}
}

void perturbationMethods::perturbate(std::vector<Route>& solution) {
	std::vector<PERTURBATIONTYPES> pert = getAll();
	int selectedPerturbation = Utils::getRandomInt(0, pert.size());
	return executePerturbation(solution, pert[selectedPerturbation]);
}