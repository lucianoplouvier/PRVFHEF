#pragma once
#include "auxiliaryStructures.h"

namespace fractionRoute {

	std::vector<Route> splitReinsertion(const std::vector<Route>& solution, const Client& client, int forbiddenRouteIndex, bool& success, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehicleTypes);

	std::vector<Route> knaapSackGreedy(std::vector<Route>& solution, const Client& client, const std::vector<int>& leastInsertPos,
		std::vector<int>& routesIndexWithResidual /*L*/, std::vector<double>& residualsList /*A*/, std::vector<double>& leastInsertionCosts /*U*/, const std::vector<Vehicle>& vehiclesList, bool& success);

	std::vector<Route> emptyRoutes(const std::vector<Route>& solution, int maxVels, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehicleTypes);

	std::vector<Route> reinsertSingleCustomer(std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehicleTypes);

}