#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "route.h"
#include "auxiliaryStructures.h"
#include "Utils.h"


/*
* @brief namespace para execução das buscas locais do Iterated Local Search.
*/
namespace intrarouteStructures {

	std::list<INTRAROUTETYPES> getAll();

	static int intrarrouteCount() { return getAll().size(); }

	static std::vector<Route> shift(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts);

	static std::vector<Route> swap(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts);

	static std::vector<Route> orOpt2(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts);

	static std::vector<Route> orOpt3(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts);

	std::vector<Route> execute(INTRAROUTETYPES type, std::vector<Route>& solution, AdjacencyCosts& adjCosts, const std::vector<Client>& clientList);

	std::vector<Route> executeRandom(std::vector<Route>& solution, AdjacencyCosts& adjCosts, const std::vector<Client>& clientList);

	static std::vector<Route> twoOPT(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts);

}