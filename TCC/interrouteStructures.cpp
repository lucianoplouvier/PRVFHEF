#include "interrouteStructures.h"

using namespace std;

void interrouteStructures::shift1_0() {

}

void interrouteStructures::shift2_0() {

}

void interrouteStructures::swap1_1() {

}

void interrouteStructures::swap2_1() {

}

void interrouteStructures::swap1_1S() {

}

void interrouteStructures::swap2_1S() {

}

void interrouteStructures::routeAddition() {

}

void interrouteStructures::kSplit() {

}

std::list<INTERROUTETYPES> interrouteStructures::getAll() {
	std::list<INTERROUTETYPES> all;
	all.push_back(INTERROUTETYPES::KSPLIT);
	all.push_back(INTERROUTETYPES::ROUTEADDITION);
	all.push_back(INTERROUTETYPES::SHIFT1_0);
	all.push_back(INTERROUTETYPES::SHIFT2_0);
	all.push_back(INTERROUTETYPES::SWAP1_1);
	all.push_back(INTERROUTETYPES::SWAP1_1S);
	all.push_back(INTERROUTETYPES::SWAP2_1);
	all.push_back(INTERROUTETYPES::SWAP2_1S);
	return all;
}

std::vector<Route> interrouteStructures::executeInterroute(INTERROUTETYPES type, std::vector<Route>& current) {
	std::vector<Route> result = current;

	return result;
}