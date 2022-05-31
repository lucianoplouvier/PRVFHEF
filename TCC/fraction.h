#pragma once
#include "auxiliaryStructures.h"

namespace fractionRoute {

	std::vector<Route> splitReinsertion(const std::vector<Route>& solution, const Client& client, int forbiddenRouteId, bool& success, const AdjacencyCosts& adjacencyCosts);

	std::vector<Route> knaapSackGreedy(std::vector<Route>& solution, const Client& client,
		std::vector<int>& routesIndexWithResidual /*L*/, std::vector<int>& residualsList /*A*/, std::vector<int>& leastInsertionCosts /*U*/, bool& success);

	std::vector<Route> emptyRoutes(const std::vector<Route>& solution, int maxVels, const AdjacencyCosts& adjacencyCosts);

	std::vector<Route> reinsertSingleCustomer(std::vector<Route>& solution, const AdjacencyCosts& adjacencyCosts);

}