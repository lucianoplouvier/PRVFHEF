#pragma once
#include "structures.h"
#include <vector>
#include "Route.h"
#include "Utils.h"

enum class PERTURBATIONTYPES {
	MULTISWAP,
	MULTISHIFT,
	SPLIT,
	MERGE
};

namespace perturbationMethods {

	std::vector<Route> multipleSwap1_1(std::vector<Route>& solution);

	std::vector<Route> multiShift1_1(std::vector<Route>& solution);

	std::vector<Route> split(std::vector<Route>& solution);

	std::vector<Route> merge(std::vector<Route>& solution);

	std::vector<PERTURBATIONTYPES> getAll();

	std::vector<Route> perturbate(std::vector<Route>& solution);

	std::vector<Route> executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type);

};