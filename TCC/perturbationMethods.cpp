#include "perturbationMethods.h"
#include "intrarouteStructures.h"

std::vector<Route> perturbationMethods::multipleSwap1_1(std::vector<Route>& solution) {
	std::vector<Route> result;



	return result;
}

std::vector<Route> perturbationMethods::multiShift1_1(std::vector<Route>& solution) {
	std::vector<Route> result;



	return result;
}

std::vector<Route> perturbationMethods::split(std::vector<Route>& solution) {
	std::vector<Route> result;



	return result;
}

std::vector<Route> perturbationMethods::merge(std::vector<Route>& solution) {
	std::vector<Route> result;



	return result;
}

std::vector<PERTURBATIONTYPES> perturbationMethods::getAll() {
	std::vector<PERTURBATIONTYPES> all;
	all.push_back(PERTURBATIONTYPES::MULTISWAP);
	all.push_back(PERTURBATIONTYPES::MULTISHIFT);
	all.push_back(PERTURBATIONTYPES::SPLIT);
	all.push_back(PERTURBATIONTYPES::MERGE);
	return all;
}

std::vector<Route> perturbationMethods::executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type) {
	if (type == PERTURBATIONTYPES::MULTISWAP) {
		return multipleSwap1_1(solution);
	}
	else if (type == PERTURBATIONTYPES::MULTISHIFT) {
		return multiShift1_1(solution);
	}
	else if (type == PERTURBATIONTYPES::SPLIT) {
		return split(solution);
	}
	else if (type == PERTURBATIONTYPES::MERGE) {
		return merge(solution);
	}
	else {
		// ERRO;
		int ashjd = 0;
		return std::vector<Route>();
	}
}

std::vector<Route> perturbationMethods::perturbate(std::vector<Route>& solution) {
	std::vector<PERTURBATIONTYPES> pert = getAll();
	int selectedPerturbation = Utils::getRandomInt(0, pert.size() - 1);
	return executePerturbation(solution, pert[selectedPerturbation]);
}