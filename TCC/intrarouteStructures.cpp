#include "intrarouteStructures.h"

using namespace std;
using namespace RouteDefs;

int intrarouteStructures::getIntrarrouteSumImprove() {
	return shiftImprove + swapImprove + orOpt2Improve + orOpt3Improve + twoOptImprove;
}

int intrarouteStructures::getShiftImprove() {
	return shiftImprove;
}

int intrarouteStructures::getSwapImprove() {
	return swapImprove;
}

int intrarouteStructures::getorOpt2Improve() {
	return orOpt2Improve;
}

int intrarouteStructures::getorOpt3Improve() {
	return orOpt3Improve;
}

int intrarouteStructures::getTwoOptImprove() {
	return twoOptImprove;
}

std::list<INTRAROUTETYPES> intrarouteStructures::getAll() {
	std::list<INTRAROUTETYPES> all;
	all.push_back(INTRAROUTETYPES::REINSERTION);
	all.push_back(INTRAROUTETYPES::EXCHANGE);
	all.push_back(INTRAROUTETYPES::OR_OPT2);
	all.push_back(INTRAROUTETYPES::OR_OPT3);
	all.push_back(INTRAROUTETYPES::TWO_OPT);
	return all;
}

std::vector<Route> intrarouteStructures::execute(INTRAROUTETYPES type, std::vector<Route>& solution, AdjacencyCosts& adjCosts, const std::vector<Client>& clientList) {
	std::vector<Route> result = solution;
	switch (type)
	{
	case INTRAROUTETYPES::EXCHANGE:
		result = shift(solution, adjCosts);
		break;
	case INTRAROUTETYPES::REINSERTION:
		result = swap(solution, adjCosts);
		break;
	case INTRAROUTETYPES::OR_OPT2:
		result = orOpt2(solution, adjCosts);
		break;
	case INTRAROUTETYPES::OR_OPT3:
		result = orOpt3(solution, adjCosts);
		break;
	case INTRAROUTETYPES::TWO_OPT:
		result = twoOPT(solution, adjCosts);
		break;
	default:
		cout << "ERROR. intrarouteStructures::execute , tipo de vizinhança não reconhecido.";
		break;
	}
	return result;
}

std::vector<Route> intrarouteStructures::executeAll(std::vector<Route>& solution, AdjacencyCosts& adjCosts, const std::vector<Client>& clientList) {
	std::list<INTRAROUTETYPES> intrarouteList = intrarouteStructures::getAll();
	for (int i = 0; i < intrarouteList.size(); i++) {
		std::list<INTRAROUTETYPES>::iterator iterator = intrarouteList.begin();
		std::advance(iterator, i);
		solution = intrarouteStructures::execute(*iterator, solution, adjCosts, clientList);
	}
	return solution;
}

Route executeShift(const Route& route, float initialEval, const AdjacencyCosts& adjacencyCosts, float& resultEval) {
	Route result(route);
	resultEval = initialEval;
	bool improvementFound = true;
	while (improvementFound) {
		improvementFound = false;
		Route bestRoute(result);
		float bestEval = resultEval;
		for (int aClient = 0; aClient < route.clientsList.size(); aClient++) {
			Route routeToChange(bestRoute);
			float changeEval = bestEval;
			for (int iClient = 0; iClient < routeToChange.clientsList.size(); iClient++) {
				Route step(routeToChange);
				Client currClient = step.clientsList[iClient];
				std::vector<Client> clients;
				clients.push_back(currClient);
				step.removeClient(currClient);
				int pos = RouteDefs::findBestInsertion(step, clients, adjacencyCosts).second;
				step.insertClient(currClient, pos);
				float executionEval = RouteDefs::evaluateRoute(step, adjacencyCosts);
				if (executionEval < resultEval) {
					changeEval = executionEval; // TESTE Confirm
					routeToChange = step;
					improvementFound = true;
				}
			}
			if (changeEval < bestEval) {
				bestEval = changeEval;
				bestRoute = routeToChange;
			}
		}
		if (bestEval < resultEval) {
			result = bestRoute;
			resultEval = bestEval;
		}
	}
	return result;
}

static std::vector<Route> intrarouteStructures::shift(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts) {
	std::vector<Route> result = solution;
	bool improved = false;
	for (int iRoute = 0; iRoute < result.size(); iRoute++) {
		Route currRoute(result[iRoute]);
		if (currRoute.clientsList.size() > 1) {
			float currentEval = RouteDefs::evaluateRoute(currRoute, adjacencyCosts);
			float bestEval;
			currRoute = executeShift(currRoute, currentEval, adjacencyCosts, bestEval);
			if (bestEval < currentEval) {
				result[iRoute] = currRoute;
				improved = true;
			}
		}
	}
	if (improved) {
		shiftImprove++;
	}
	return result;
}

void invertRoute(Route& route, int indexStart, int indexEnd) {
	while (indexStart < indexEnd) {
		Client i = route.clientsList[indexStart];
		Client j = route.clientsList[indexEnd];
		route.clientsList[indexStart] = j;
		route.clientsList[indexEnd] = i;
		indexStart++;
		indexEnd--;
	}
}

Route executeSwap(const Route& route, float initialEval, const AdjacencyCosts& adjacencyCosts, float& resultEval) {
	Route result = route;
	resultEval = initialEval;
	bool improvementFound = true;
	while (improvementFound) {
		improvementFound = false;
		Route bestRoute = result;
		float bestEval = resultEval;
		for (int aClient = 0; aClient < route.clientsList.size(); aClient++) {
			Route routeToChange(bestRoute);
			for (int iClient = 0; iClient < routeToChange.clientsList.size(); iClient++) {
				int jClient = iClient + 1;
				for (jClient; jClient < routeToChange.clientsList.size(); jClient++) {
					Route afterStep = routeToChange;
					invertRoute(afterStep, iClient, jClient);
					float executionEval = RouteDefs::evaluateRoute(afterStep, adjacencyCosts);
					if (executionEval < bestEval) {
						bestEval = executionEval;
						bestRoute = afterStep;
						improvementFound = true;
					}
				}
			}
		}
		if (bestEval < resultEval) {
			result = bestRoute;
			resultEval = bestEval;
		}
	}
	return result;

}

// Tem que inverter a direção entre um cliente e o outro trocados
static std::vector<Route> intrarouteStructures::swap(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts) {
	std::vector<Route> result = solution;
	bool improved = false;
	for (int i = 0; i < solution.size(); i++) {
		Route currentRoute = result[i];
		float currentEval = RouteDefs::evaluateRoute(currentRoute, adjacencyCosts);
		float eval;
		if (currentRoute.clientsList.size() > 1) {
			executeSwap(currentRoute, currentEval, adjacencyCosts, eval);
			if (eval < currentEval) {
				result[i] = currentRoute;
				improved = true;
			}
		}
	}
	if (improved) {
		swapImprove++;
	}
	return result;
}

Route executeOP2(const Route& route, float initialEval, const AdjacencyCosts& adjacencyCosts, float& resultEval) {
	Route result(route);
	resultEval = initialEval;
	bool improvementFound = true;
	while (improvementFound) {
		improvementFound = false;
		Route bestRoute(result);
		float bestEval = resultEval;
		for (int aClient = 0; aClient < route.clientsList.size(); aClient++) {
			Route routeToChange(bestRoute);
			for (int iClient = 0; iClient < routeToChange.clientsList.size() - 1; iClient++) {
				Route step = routeToChange;
				Client currClient = step.clientsList[iClient];
				Client nextClient = step.clientsList[iClient + 1];
				std::vector<Client> clients;
				clients.push_back(currClient);
				clients.push_back(nextClient);
				step.removeClient(currClient);
				step.removeClient(nextClient);
				int pos = RouteDefs::findBestInsertion(step, clients, adjacencyCosts).second;
				step.insertClient(nextClient, pos);
				step.insertClient(currClient, pos);
				float executionEval = RouteDefs::evaluateRoute(step, adjacencyCosts);
				if (executionEval < bestEval) {
					bestEval = executionEval;
					bestRoute = step;
					improvementFound = true;
				}
			}
		}
		if (bestEval < resultEval) {
			result = bestRoute;
			resultEval = bestEval;
		}
	}
	return result;
}

static std::vector<Route> intrarouteStructures::orOpt2(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts) {
	std::vector<Route> result = solution;
	bool improved = false;
	for (int iRoute = 0; iRoute < result.size(); iRoute++) {
		Route currRoute = result[iRoute];
		float currentEval = RouteDefs::evaluateRoute(currRoute, adjacencyCosts);
		if (currRoute.clientsList.size() > 2) {
			float resultEval = currentEval + 1;
			currRoute = executeOP2(currRoute, currentEval, adjacencyCosts, resultEval);
			if (resultEval < currentEval) {
				result[iRoute] = currRoute;
				improved = true;
			}
		}
	}
	if (improved) {
		orOpt2Improve++;
	}
	return result;
}

Route executeOP3(const Route& route, float initialEval, const AdjacencyCosts& adjacencyCosts, float& resultEval) {
	Route result(route);
	resultEval = initialEval;
	bool improvementFound = true;
	while (improvementFound) {
		improvementFound = false;
		Route bestRoute(result);
		float bestEval = resultEval;
		for (int aClient = 0; aClient < route.clientsList.size(); aClient++) {
			Route routeToChange(bestRoute);
			for (int iClient = 0; iClient < routeToChange.clientsList.size() - 2; iClient++) {
				Route step(routeToChange);
				Client currClient = step.clientsList[iClient];
				Client nextClient = step.clientsList[iClient + 1];
				Client lastClient = step.clientsList[iClient + 2];
				step.removeClient(currClient);
				step.removeClient(nextClient);
				step.removeClient(lastClient);
				std::vector<Client> clients;
				clients.push_back(currClient);
				clients.push_back(nextClient);
				clients.push_back(lastClient);
				int pos = RouteDefs::findBestInsertion(step, clients, adjacencyCosts).second;
				step.insertClient(lastClient, pos);
				step.insertClient(nextClient, pos);
				step.insertClient(currClient, pos);
				float executionEval = RouteDefs::evaluateRoute(step, adjacencyCosts);
				if (executionEval < bestEval) {
					bestEval = executionEval;
					bestRoute = step;
					improvementFound = true;
				}
			}
		}
		if (bestEval < resultEval) {
			result = bestRoute;
			resultEval = bestEval;
		}
	}
	return result;
}

static std::vector<Route> intrarouteStructures::orOpt3(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts) {
	std::vector<Route> result = solution;
	bool improved = false;
	for (int iRoute = 0; iRoute < result.size(); iRoute++) {
		Route currRoute = result[iRoute];
		float currentEval = RouteDefs::evaluateRoute(currRoute, adjacencyCosts);
		if (currRoute.clientsList.size() > 3) {
			float resultEval = currentEval + 1;
			currRoute = executeOP3(currRoute, currentEval, adjacencyCosts, resultEval);
			if (resultEval < currentEval) {
				result[iRoute] = currRoute;
				improved = true;
			}
		}
	}
	if (improved) {
		orOpt3Improve++;
	}
	return result;
}

Route executeTwoOpt(const Route& route, float initialEval, const AdjacencyCosts& adjacencyCosts, float& resultEval) {
	Route result(route);
	if (route.clientsList.size() < 2) return result;
	bool improvementFound = true;
	resultEval = initialEval;
	while (improvementFound) {
		improvementFound = false;
		Route bestRoute(result);
		float bestEval = resultEval;
		for (int i = 0; i < route.clientsList.size() - 2; i++) {
			for (int j = i + 2; j < route.clientsList.size() - 1; j++) {
				Route currRoute(result);
				invertRoute(currRoute, i + 1, j + 1);
				float eval = RouteDefs::evaluateRoute(currRoute, adjacencyCosts);
				if (eval < bestEval) {
					bestEval = eval;
					improvementFound = true;
					bestRoute = currRoute;
				}
			}
		}
		if (bestEval < resultEval) {
			result = bestRoute;
			resultEval = bestEval;
		}
	}
	return result;
}

static std::vector<Route> intrarouteStructures::twoOPT(std::vector<Route>& solution, AdjacencyCosts & adjacencyCosts) {
	std::vector<Route> result = solution;
	bool improved = false;
	for (int iRoute = 0; iRoute < result.size(); iRoute++) {
		Route currRoute = result[iRoute];
		float currentEval = RouteDefs::evaluateRoute(currRoute, adjacencyCosts);
		if (currRoute.clientsList.size() > 3) {
			float resultEval = currentEval;
			currRoute = executeTwoOpt(currRoute, currentEval, adjacencyCosts, resultEval);
			if (resultEval < currentEval) {
				result[iRoute] = currRoute;
				currentEval = resultEval;
				improved = true;
			}
		}
	}
	if (improved) {
		twoOptImprove++;
	}
	return result;
}