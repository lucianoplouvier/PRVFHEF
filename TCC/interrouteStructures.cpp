#include "interrouteStructures.h"

using namespace std;

std::vector<Route> interrouteStructures::shift1_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	RouteDefs::isSolutionValid(solution, originalClients);
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		Route routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			if (j == 3) {
				int aisjd = 0;
			}
			Route routeJ = solution[j];
			if (routeI.id != routeJ.id) {
				bool nStatus = auxStruct->neighborhoodStatus(INTERROUTETYPES::SHIFT1_0, routeI.id);
				float minDeliveryI = auxStruct->minDelivery(i);
				float sumDeliveryJ = auxStruct->sumDelivery(j);
				if (nStatus && (minDeliveryI + sumDeliveryJ) <= routeJ.vehicle.capacity) {
					for (int clientI = 0; clientI < routeI.clientsList.size(); clientI++) {
						Client client = routeI.clientsList[clientI];
						if (client.demand + sumDeliveryJ <= routeJ.vehicle.capacity) { // Mover de routeI para routeJ
							if (solution[j].findClient(client.id) < 0) { // Se não tem o cliente na rota j prosseguir
								std::vector<Client> clients;
								clients.push_back(client);
								int pos = RouteDefs::findBestInsertion(routeJ, clients, adjCosts).second;
								std::vector<Route> newResult = RouteDefs::copy(solution);
								newResult[i].removeClient(client);
								bool ok = newResult[j].insertClient(client, pos);
								if (ok) {
									float eval = RouteDefs::evaluate(newResult, adjCosts);
									if (eval < resultEval && RouteDefs::isSolutionValid(newResult, originalClients)) {
										result = newResult;
										resultEval = eval;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (resultEval < evaluation) {
		//auxStruct->improvementChanged(INTERROUTETYPES::SHIFT1_0, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::shift2_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < result.size(); i++) {
		Route& routeI = solution[i];
		if (routeI.clientsList.size() > 1) {
			for (int j = 0; j < solution.size(); j++) {
				Route& routeJ = solution[j];
				if (routeI.id != routeJ.id) {
					bool nStatus = auxStruct->neighborhoodStatus(INTERROUTETYPES::SHIFT2_0, routeI.id);
					float minDeliveryI = auxStruct->minDelivery(i);
					float sumDeliveryJ = auxStruct->sumDelivery(j);
					if (nStatus && (minDeliveryI + sumDeliveryJ) <= routeJ.vehicle.capacity) {
						for (int clientI = 0; clientI < routeI.clientsList.size() - 1; clientI++) {
							Client client = routeI.clientsList[clientI];
							Client next = routeI.clientsList[clientI + 1];
							std::vector<Client> clients;
							clients.push_back(client);
							clients.push_back(next);
							if (client.demand + sumDeliveryJ <= routeJ.vehicle.capacity) { // Mover de routeI para routeJ
								if (solution[j].findClient(client.id) < 0 && solution[j].findClient(next.id) < 0) { // Se não tem o cliente na rota j prosseguir
									int pos = RouteDefs::findBestInsertion(routeJ, clients, adjCosts).second;
									std::vector<Route> newResult = RouteDefs::copy(solution);
									newResult[i].removeClient(next);
									newResult[i].removeClient(client);
									bool ok = newResult[j].insertClient(next, pos);
									if (ok) {
										ok = newResult[j].insertClient(client, pos);
										if (ok) {
											float eval = RouteDefs::evaluate(newResult, adjCosts);
											if (eval < resultEval && RouteDefs::isSolutionValid(newResult, originalClients)) {
												result = newResult;
												resultEval = eval;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (resultEval < evaluation) {
		//auxStruct->improvementChanged(INTERROUTETYPES::SHIFT1_0, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::swap1_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		Route& routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			Route& routeJ = solution[j];
			if (routeI.id != routeJ.id) {
				bool nStatusI = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP1_1, routeI.id);
				bool nStatusJ = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP1_1, routeI.id);
				float minDeliveryI = auxStruct->minDelivery(i);
				float maxDeliveryJ = auxStruct->maxDelivery(j);
				float sumDeliveryJ = auxStruct->sumDelivery(j);
				float routeJCapacity = routeJ.vehicle.capacity;
				if ((nStatusI || nStatusJ) && (minDeliveryI - maxDeliveryJ + sumDeliveryJ <= routeJCapacity)) {
					int routeISize = routeI.clientsList.size();
					if (routeISize > 0) {
						for (int iClient = 0; iClient < routeISize; iClient++) {
							Client& client = routeI.clientsList[iClient];
							if (client.demand + sumDeliveryJ - maxDeliveryJ <= routeJCapacity) {
								for (int jClient = 0; jClient < routeJ.clientsList.size(); jClient++) {
									Client& other = routeJ.clientsList[jClient];
									if (routeI.findClient(other.id) < 0 && routeJ.findClient(client.id) < 0) { // Verifica se contém
										std::vector<Route> newResult = RouteDefs::copy(solution);
										RouteDefs::swapClients(newResult[i], client.id, newResult[j], other.id);
										if (RouteDefs::isSolutionValid(newResult, originalClients)) {
											float eval = RouteDefs::evaluate(newResult, adjCosts);
											if (eval < resultEval) {
												resultEval = eval;
												result = newResult;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (resultEval < evaluation) {
		//auxStruct->improvementChanged(INTERROUTETYPES::SWAP1_1, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::swap2_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	return solution;
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		Route& routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			Route& routeJ = solution[j];
			if (routeI.id != routeJ.id) {
				bool nStatusI = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP2_1, routeI.id);
				bool nStatusJ = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP2_1, routeI.id);
				float minDeliveryI = auxStruct->minDelivery(i);
				float maxDeliveryJ = auxStruct->maxDelivery(j);
				float sumDeliveryJ = auxStruct->sumDelivery(j);
				float routeJCapacity = routeJ.vehicle.capacity;
				if ((nStatusI || nStatusJ) && (minDeliveryI - maxDeliveryJ + sumDeliveryJ <= routeJCapacity)) {
					int routeISize = routeI.clientsList.size();
					if (routeISize > 0) {
						for (int iClient = 0; iClient < routeISize - 1; iClient++) {
							Client& client = routeI.clientsList[iClient];
							Client& next = routeI.clientsList[iClient + 1];
							if (client.demand + sumDeliveryJ - maxDeliveryJ <= routeJCapacity) {
								for (int jClient = 0; jClient < routeJ.clientsList.size(); jClient++) {
									Client& other = routeJ.clientsList[jClient];
									std::vector<Route> newResult = RouteDefs::copy(solution);
									RouteDefs::swapClients(newResult[i], iClient, iClient + 1, newResult[j], other.id);
									float eval = RouteDefs::evaluate(newResult, adjCosts);
									if (eval < resultEval && RouteDefs::isSolutionValid(newResult, originalClients)) {
										resultEval = eval;
										result = newResult;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (resultEval < evaluation) {
		//auxStruct->improvementChanged(INTERROUTETYPES::SWAP1_1, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::swap1_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts) {
	exit(1);
}

std::vector<Route> interrouteStructures::swap2_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts) {
	exit(1);
}

std::vector<Route> interrouteStructures::cross(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<Route> result = RouteDefs::copy(solution);
	float resultEval = evaluation;
	for (int iRoute = 0; iRoute < result.size(); iRoute++) {
		Route routeI = solution[iRoute];
		for (int jRoute = 0; jRoute < result.size(); jRoute++) {
			if (iRoute != jRoute) {
				Route routeJ = solution[jRoute];
				int routeISize = routeI.clientsList.size();
				int routeJSize = routeJ.clientsList.size();
				for (int iClient = 1; iClient < routeISize; iClient++) {
					for (int jClient = 1; jClient < routeJSize; jClient++) {
						float demandI = routeI.getDemand(iClient, routeI.clientsList.size() - 1);
						float demandJ = routeJ.getDemand(jClient, routeJ.clientsList.size() - 1);
						if (routeI.getTotalDemand() - demandI + demandJ <= routeI.vehicle.capacity &&
							routeJ.getTotalDemand() - demandJ + demandI <= routeJ.vehicle.capacity) {
							bool proceed = true;
							for (int checkI = 0; checkI < routeISize; checkI++) { // Verifica se a outra rota já tem um cliente antes de prosseguir
								if (routeJ.findClient(routeI.clientsList[checkI].id) != -1) {
									proceed = false;
									break;
								}
							}
							for (int checkJ = 0; checkJ < routeJSize; checkJ++) { // Verifica se a outra rota já tem um cliente antes de prosseguir
								if (routeI.findClient(routeJ.clientsList[checkJ].id) != -1) {
									proceed = false;
									break;
								}
							}
							if (proceed) {
								std::vector<Client> newClientIList, newClientJList;
								for (int i = 0; i < iClient; i++) {
									newClientIList.push_back(routeI.clientsList[i]);
								}
								for (int i = jClient; i < routeJSize; i++) {
									newClientIList.push_back(routeJ.clientsList[i]);
								}

								for (int i = 0; i < jClient; i++) {
									newClientJList.push_back(routeJ.clientsList[i]);
								}
								for (int i = iClient; i < routeISize; i++) {
									newClientJList.push_back(routeI.clientsList[i]);
								}
								std::vector<Route> newResult = RouteDefs::copy(solution);
								newResult[iRoute].clientsList = newClientIList;
								newResult[jRoute].clientsList = newClientJList;
								float eval = RouteDefs::evaluate(newResult, adjCosts);
								if (eval < resultEval && RouteDefs::isSolutionValid(newResult, originalClients)) {
									resultEval = eval;
									result = newResult;
								}
							}
						}
					}
				}
			}
		}
	}
	return result;
}

std::vector<Route> interrouteStructures::routeAddition(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts) {
	exit(1);
}

std::vector<Route> interrouteStructures::kSplit(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts) {
	exit(1);
}

std::list<INTERROUTETYPES> interrouteStructures::getAll() {
	std::list<INTERROUTETYPES> all;
	//all.push_back(INTERROUTETYPES::KSPLIT);
	//all.push_back(INTERROUTETYPES::ROUTEADDITION);
	all.push_back(INTERROUTETYPES::SHIFT1_0);
	all.push_back(INTERROUTETYPES::SHIFT2_0);
	all.push_back(INTERROUTETYPES::SWAP1_1);
	all.push_back(INTERROUTETYPES::SWAP2_1);
	all.push_back(INTERROUTETYPES::CROSS);
	//all.push_back(INTERROUTETYPES::SWAP1_1S);
	//all.push_back(INTERROUTETYPES::SWAP2_1S);
	return all;
}

std::vector<Route> interrouteStructures::executeInterroute(INTERROUTETYPES type, std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	switch (type)
	{
	case INTERROUTETYPES::SHIFT1_0:
		return shift1_0(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::SHIFT2_0:
		return shift2_0(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::SWAP1_1:
		return swap1_1(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::SWAP2_1:
		return swap2_1(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::SWAP1_1S:
		return swap1_1S(solution, evaluation, auxStruct, adjCosts);
		break;
	case INTERROUTETYPES::SWAP2_1S:
		return swap2_1(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::ROUTEADDITION:
		return routeAddition(solution, evaluation, auxStruct, adjCosts);
		break;
	case INTERROUTETYPES::KSPLIT:
		return kSplit(solution, evaluation, auxStruct, adjCosts);
		break;
	case INTERROUTETYPES::CROSS:
		return cross(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	default:
		cout << "Erro ao escolher interrota.";
		exit(1);
		break;
	}
}