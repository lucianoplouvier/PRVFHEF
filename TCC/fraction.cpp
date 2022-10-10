#include "fraction.h"

std::vector<Route> fractionRoute::splitReinsertion(const std::vector<Route>& solution, const Client& client, int forbiddenRouteIndex, bool& success, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehicleTypes) {
	std::vector<int> routesIndexWithResidual; // L
	std::vector<double> residualsList; // A
	std::vector<double> leastInsertionCosts; // U
	std::vector<int> leastInsertionPos;


	std::vector<Route> result = solution;

	int vehicles = solution.size();
	double totalResidual = 0;
	for (int i = 0; i < vehicles; i++) {
		const Route& r = solution[i];
		if (!r.vehicle.ficticous) {
			double rTotalDemand = r.getTotalDemand();
			double currResidual = r.vehicle.capacity - rTotalDemand;
			if (i != forbiddenRouteIndex && currResidual > 0) {
				routesIndexWithResidual.push_back(i);
				residualsList.push_back(currResidual);
				totalResidual += currResidual;
			}
		}
	}

	if (totalResidual >= client.demand) { // Então existe o espaço para o cliente.
		for (int i = 0; i < routesIndexWithResidual.size(); i++) {
			int routeIndex = routesIndexWithResidual[i];
			if (!result[routeIndex].vehicle.ficticous) {
				std::vector<Client> clients;
				clients.push_back(client);
				auto bestInsert = RouteDefs::findBestInsertion(result[routeIndex], clients, adjacencyCosts);
				leastInsertionCosts.push_back(bestInsert.first);
				leastInsertionPos.push_back(bestInsert.second);
			}
		}

		if (residualsList.size() != leastInsertionCosts.size()) {
			cout << "PRVFHEF::splitReinsertion , erro de tamanhos para knaapsack";
			exit(1);
		}

		return knaapSackGreedy(result, client, leastInsertionPos, routesIndexWithResidual, residualsList, leastInsertionCosts, vehicleTypes, success);
	}
	return solution;
}

std::vector<Route> fractionRoute::knaapSackGreedy(std::vector<Route>& solution, const Client& client, const std::vector<int>& leastInsertPos,
	std::vector<int>& routesIndexWithResidual /*L*/, std::vector<double>& residualsList /*A*/, std::vector<double>& leastInsertionCosts /*U*/, const std::vector<Vehicle>& vehicleTypes, bool& success) {

	double currDemand = client.demand;

	struct Profit {
		int id;
		double value;
	};

	double avgCapacity = 0;
	double maxCapacity = 0;
	int countVehicle = 0;
	for (int i = 0; i < vehicleTypes.size(); i++) {
		countVehicle++;
		avgCapacity += vehicleTypes[i].capacity;
		if (vehicleTypes[i].capacity > maxCapacity) {
			maxCapacity = vehicleTypes[i].capacity;
		}
	}
	avgCapacity /= countVehicle;

	std::vector<Profit> profits(residualsList.size());

	for (int i = 0; i < residualsList.size(); i++) {
		double& residual = residualsList[i];
		double& cost = leastInsertionCosts[i];
		double profitval = cost / residual;
		double velCapacity = solution[routesIndexWithResidual[i]].vehicle.capacity;
		if (velCapacity < avgCapacity) {
			double diff = std::abs(std::abs(velCapacity) - std::abs(avgCapacity));
			diff = diff / maxCapacity;
			profitval = profitval + profitval * diff;
		}
		else if (velCapacity > avgCapacity) {
			double diff = std::abs(std::abs(velCapacity) - std::abs(avgCapacity));
			diff = diff / maxCapacity;
			profitval = profitval - profitval * diff;
		}

		Profit p;
		p.id = i;
		p.value = profitval;
		profits[i] = p;
	}

	std::sort(profits.begin(), profits.end(), [](const Profit& p, const Profit& q) { return (double)p.value < (double)q.value; });
	int i = 0;
	for (i; i < solution.size() && currDemand > 0; i++) {
		Profit p = profits[i];
		int chosenRouteIndex = routesIndexWithResidual[p.id]; // The chosen route is the one that has residual space for insertion.
		double residual = residualsList[p.id];
		if (residual > 0) {
			Route& chosenRoute = solution[chosenRouteIndex];
			double maxThatCanInsert = std::min(currDemand, residual);
			if (maxThatCanInsert <= 0) {
				cout << "ERROR. PRVFHEF::knaapSackGreedy, Trying to add demand <= 0.";
			}
			if (chosenRoute.canAddClient(maxThatCanInsert)) {
				Client maxPossible;
				maxPossible.id = client.id;
				maxPossible.demand = maxThatCanInsert;
				chosenRoute.insertClient(maxPossible, leastInsertPos[p.id]);
				currDemand -= maxThatCanInsert;
				residualsList[p.id] -= maxThatCanInsert;
			}
			else {
				cout << "ERROR. PRVFHEF::knaapSackGreedy, Cannot add client to best profit route.";
			}
		}
	}
	if (i >= solution.size() && (currDemand > 0 || currDemand < 0)) {
		cout << "ERROR. PRVFHEF::knaapSackGreedy, Client still not fully added.";
	}
	else {
		success = true;
	}
	return solution;
}

std::vector<Route> fractionRoute::emptyRoutes(const std::vector<Route>& solution, int maxVels, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehicleTypes) {
	//return solution;
	std::vector<Route> resultSolution(solution);
	int currRoutes = 0; // Só contar rotas não vazias.
	for (const Route& r : resultSolution) {
		if (r.clientsList.empty()) {
			currRoutes++;
		}
	}

	int tries = 0;
	while (currRoutes > maxVels && tries < resultSolution.size()) {
		int mostEmptyRouteIndex = -1;
		double cargo = std::numeric_limits<double>::max();
		for (int i = 0; i < resultSolution.size(); i++) {
			if (!resultSolution[i].clientsList.empty()) {
				double currCargo = resultSolution[i].getTotalDemand();
				if (currCargo < cargo) {
					cargo = currCargo;
					mostEmptyRouteIndex = i;
				}
			}
		}
		if (mostEmptyRouteIndex != -1) {
			for (int i = 0; i < resultSolution[mostEmptyRouteIndex].clientsList.size(); i++) {
				Client c(resultSolution[mostEmptyRouteIndex].clientsList[i]);
				bool success = false;
				resultSolution = splitReinsertion(resultSolution, c, mostEmptyRouteIndex, success, adjacencyCosts, vehicleTypes); // Aqui pode ser só o cliente pois queremos esvaziar a rota.
				if (success) {
					resultSolution[mostEmptyRouteIndex].removeClient(c);
					i--;
				}
			}
			if (resultSolution[mostEmptyRouteIndex].clientsList.size() == 0) {
				currRoutes--;
			}
			tries++;
		}
		else {
			cout << "ERROR. fractionRoute::emptyRoutes , não encontrou o índice de menor conteúdo.";
			exit(1);
		}
	}
	return resultSolution;
}

std::vector<Route> fractionRoute::reinsertSingleCustomer(std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehicleTypes) {
	bool splitApplied = false;
	std::vector<Route> result = solution;
	double resultEval = RouteDefs::evaluate(solution, adjacencyCosts);
	do {
		splitApplied = false;

		std::vector<Route> stepSol = result;
		double stepEval = resultEval;

		for (int i = 0; i < stepSol.size(); i++) {
			Route& r = stepSol[i];
			if (r.clientsList.size() == 1) {
				Client c = r.takeClient(0);
				bool success = false;
				std::vector<Route> splitSol = splitReinsertion(stepSol, c, i, success, adjacencyCosts, vehicleTypes); // Aqui pode ser só c pois só queremos esvaziar a rota.
				if (success) {
					double solEval = RouteDefs::evaluate(splitSol, adjacencyCosts);
					if (solEval < stepEval) {
						stepSol = splitSol;
						stepEval = solEval;
						splitApplied = true;
					}
					else {
						r.clientsList.push_back(c);
					}
				}
				else {
					r.clientsList.push_back(c);
				}
			}
		}
		if (splitApplied) {
			result = stepSol;
			resultEval = stepEval;
		}
	} while (splitApplied);
	return result;
}