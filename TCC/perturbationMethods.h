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

namespace perturbationMethods { // Não retorna nada pois ele verifica antes de fazer movimentos inválidos, aí altera diretamente.

	std::vector<PERTURBATIONTYPES> getAll();

	void multipleSwap1_1(std::vector<Route>& solution);

	void multiShift1_1(std::vector<Route>& solution);

	void split(std::vector<Route>& solution);

	void merge(std::vector<Route>& solution);

	void perturbate(std::vector<Route>& solution);

	void executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type);

};