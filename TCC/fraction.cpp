#include "fraction.h"

std::vector<Route> fractionRoute::splitReinsertion(const std::vector<Route>& solution, const Client& client, int forbiddenRouteIndex, bool& success, const AdjacencyCosts& adjacencyCosts) {
	std::vector<int> routesIndexWithResidual; // L
	std::vector<float> residualsList; // A
	std::vector<float> leastInsertionCosts; // U

	std::vector<Route> result = solution;

	int vehicles = solution.size();
	float totalResidual = 0;
	for (int i = 0; i < vehicles; i++) {
		const Route& r = solution[i];
		float rTotalDemand = r.getTotalDemand();
		float currResidual = r.vehicle.capacity - rTotalDemand;
		if (i != forbiddenRouteIndex && currResidual > 0) {
			routesIndexWithResidual.push_back(i);
			residualsList.push_back(currResidual);
			totalResidual += currResidual;
		}
	}

	if (totalResidual > client.demand) { // Então existe o espaço para o cliente.
		for (int i = 0; i < routesIndexWithResidual.size(); i++) {
			std::vector<Client> clients;
			clients.push_back(client);
			float leastCostToInsert = RouteDefs::findBestInsertion(result[i], clients, adjacencyCosts).first;
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

std::vector<Route> fractionRoute::knaapSackGreedy(std::vector<Route>& solution, const Client& client,
	std::vector<int>& routesIndexWithResidual /*L*/, std::vector<float>& residualsList /*A*/, std::vector<float>& leastInsertionCosts /*U*/, bool& success) {

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
		profits[i] = p;
	}

	std::sort(profits.begin(), profits.end(), [](const Profit& p, const Profit& q) { return (float)p.value > (float)q.value; });
	int i = 0;
	while (currDemand > 0 && i < solution.size()) {
		Profit p = profits[i];
		int chosenRouteIndex = routesIndexWithResidual[p.id]; // The chosen route is the one that has residual space for insertion.
		float residual = residualsList[p.id];
		if (residual > 0) {
			Route& chosenRoute = solution[chosenRouteIndex];
			float maxThatCanInsert = std::min(currDemand, residual);
			if (maxThatCanInsert <= 0) {
				cout << "ERROR. PRVFHEF::knaapSackGreedy, Trying to add demand <= 0.";
			}
			if (chosenRoute.canAddClient(maxThatCanInsert)) {
				chosenRoute.addClientOrMerge(client.id, maxThatCanInsert);
				currDemand -= maxThatCanInsert;
			}
			else {
				cout << "ERROR. PRVFHEF::knaapSackGreedy, Cannot add client to best profit route.";
			}
		}
		i++;
	}
	if (i >= solution.size() && (currDemand > 0 || currDemand < 0)) {
		cout << "WARN. PRVFHEF::knaapSackGreedy, Client still not fully added.";
	}
	else {
		success = true;
	}
	return solution;
}

std::vector<Route> fractionRoute::emptyRoutes(const std::vector<Route>& solution, int maxVels, const AdjacencyCosts& adjacencyCosts, const std::vector<Client>& clientList) {
	return solution;
	std::vector<Route> resultSolution = solution;
	int tries = 0;
	while (resultSolution.size() > maxVels && tries < resultSolution.size()) {
		int mostEmptyRouteIndex = -1;
		float cargo = std::numeric_limits<float>::max();
		for (int i = 0; i < resultSolution.size(); i++) {
			int currCargo = resultSolution[i].getTotalDemand();
			if (currCargo < cargo) {
				cargo = currCargo;
				mostEmptyRouteIndex = i;
			}
		}
		if (mostEmptyRouteIndex != -1) {
			Route& chosen = resultSolution[mostEmptyRouteIndex];
			for (int i = 0; i < chosen.clientsList.size(); i++) {
				bool success = false;
				resultSolution = splitReinsertion(resultSolution, chosen.clientsList[i], mostEmptyRouteIndex, success, adjacencyCosts); // Aqui pode ser só o cliente pois queremos esvaziar a rota.
				if (success) {
					chosen.removeClient(chosen.clientsList[i]);
					i--;
				}
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

std::vector<Route> fractionRoute::reinsertSingleCustomer(std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts, const std::vector<Client>& clientList) {
	bool splitApplied = false;
	std::vector<Route> result = solution;
	int resultEval = RouteDefs::evaluate(solution, adjacencyCosts);
	do {
		splitApplied = false;

		std::vector<Route> stepSol = result;
		int stepEval = resultEval;

		for (int i = 0; i < stepSol.size(); i++) {
			Route& r = stepSol[i];
			if (r.clientsList.size() == 1) {
				Client c = r.takeClient(0);
				bool success = false;
				std::vector<Route> splitSol = splitReinsertion(stepSol, c, i, success, adjacencyCosts); // Aqui pode ser só c pois só queremos esvaziar a rota.
				if (success) {
					int solEval = RouteDefs::evaluate(splitSol, adjacencyCosts);
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