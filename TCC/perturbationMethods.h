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

namespace perturbationMethods { // N�o retorna nada pois ele verifica antes de fazer movimentos inv�lidos, a� altera diretamente.

	std::vector<PERTURBATIONTYPES> getAll();

	void multipleSwap1_1(std::vector<Route>& solution);

	void multiShift1_1(std::vector<Route>& solution);

	void split(std::vector<Route>& solution);

	void merge(std::vector<Route>& solution);

	void perturbate(std::vector<Route>& solution);

	void executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type);

};