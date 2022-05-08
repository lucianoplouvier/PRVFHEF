#pragma once
#include "structures.h"
#include "structures.h"
#include "Route.h"
#include <vector>

enum class INTERROUTETYPES {
	SHIFT1_0,
	SHIFT2_0,
	SWAP1_1,
	SWAP2_1,
	SWAP1_1S,
	SWAP2_1S,
	ROUTEADDITION,
	KSPLIT
};

namespace interrouteStructures {

	void shift1_0();

	void shift2_0();

	void swap1_1();

	void swap2_1();

	void swap1_1S();

	void swap2_1S();

	void routeAddition();

	void kSplit();

	std::list<INTERROUTETYPES> getAll();

	std::vector<Route> executeInterroute(INTERROUTETYPES type, std::vector<Route>& current);
}