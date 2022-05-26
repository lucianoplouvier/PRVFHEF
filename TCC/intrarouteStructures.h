#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "route.h"
#include "auxiliaryStructures.h"


/*
* @brief namespace para execu��o das buscas locais do Iterated Local Search.
*/
namespace intrarouteStructures {

	std::list<INTRAROUTETYPES> getAll();

	static int intrarrouteCount() { return getAll().size(); }

	static std::vector<Route> shift(std::vector<Route>& solution, const AuxiliaryStructures* auxStruct, AdjacencyCosts& adjacencyCosts);

	static std::vector<Route> swap(std::vector<Route>& solution, const AuxiliaryStructures* auxStruct, AdjacencyCosts& adjacencyCosts);

	static std::vector<Route> orOpt2(std::vector<Route>& solution, const AuxiliaryStructures* auxStruct, AdjacencyCosts& adjacencyCosts);

	static std::vector<Route> orOpt3(std::vector<Route>& solution, const AuxiliaryStructures* auxStruct, AdjacencyCosts& adjacencyCosts);

	std::vector<Route> execute(INTRAROUTETYPES type, std::vector<Route>& solution, const AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts);

}