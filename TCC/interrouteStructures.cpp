#include "interrouteStructures.h"

using namespace std;

int interrouteStructures::getInterrouteSumImprove() {
	int total = shift10Improve + shift20Improve + swap11Improve + swap21Improve + swap11SImprove + swap21SImprove + crossImprove + kSplitImprove;
	return total;
}

int interrouteStructures::getShift10Improve() {
	return shift10Improve;
}

int interrouteStructures::getShift20Improve() {
	return shift20Improve;
}

int interrouteStructures::getSwap11Improve() {
	return swap11Improve;
}

int interrouteStructures::getSwap21Improve() {
	return swap21Improve;
}

int interrouteStructures::getSwap11SImprove() {
	return swap11SImprove;
}

int interrouteStructures::getSwap21SImprove() {
	return swap21SImprove;
}

int interrouteStructures::getCrossImprove() {
	return crossImprove;
}

int interrouteStructures::getKSplitImprove() {
	return kSplitImprove;
}

std::list<INTERROUTETYPES> interrouteStructures::getAll() {
	std::list<INTERROUTETYPES> all;
	all.push_back(INTERROUTETYPES::KSPLIT);
	//all.push_back(INTERROUTETYPES::ROUTEADDITION);
	all.push_back(INTERROUTETYPES::SHIFT1_0);
	all.push_back(INTERROUTETYPES::SHIFT2_0);
	all.push_back(INTERROUTETYPES::SWAP1_1);
	all.push_back(INTERROUTETYPES::SWAP2_1);
	all.push_back(INTERROUTETYPES::CROSS);
	all.push_back(INTERROUTETYPES::SWAP1_1S);
	all.push_back(INTERROUTETYPES::SWAP2_1S);
	return all;
}

std::vector<Route> interrouteStructures::shift1_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
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
								std::pair<int, int> costPos = RouteDefs::findBestInsertion(routeJ, clients, adjCosts);
								int pos = costPos.second;
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
							else {

							}
						}
					}
				}
			}
		}
	}
	if (resultEval < evaluation) {
		shift10Improve++;
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
		const Route& routeI = solution[i];
		if (routeI.clientsList.size() > 1) {
			for (int j = 0; j < solution.size(); j++) {
				const Route& routeJ = solution[j];
				if (routeI.id != routeJ.id) {
					bool nStatus = auxStruct->neighborhoodStatus(INTERROUTETYPES::SHIFT2_0, routeI.id);
					float minDeliveryI = auxStruct->minDelivery(i);
					float sumDeliveryJ = auxStruct->sumDelivery(j);
					if (nStatus && (minDeliveryI + sumDeliveryJ) <= routeJ.vehicle.capacity) {
						for (int clientI = 0; clientI < routeI.clientsList.size() - 1; clientI++) {
							const Client& client = routeI.clientsList[clientI];
							const Client& next = routeI.clientsList[clientI + 1];
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
		shift20Improve++;
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
		const Route& routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			const Route& routeJ = solution[j];
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
							const Client& client = routeI.clientsList[iClient];
							if (client.demand + sumDeliveryJ - maxDeliveryJ <= routeJCapacity) {
								for (int jClient = 0; jClient < routeJ.clientsList.size(); jClient++) {
									const Client& other = routeJ.clientsList[jClient];
									if (routeI.findClient(other.id) < 0 && routeJ.findClient(client.id) < 0) { // TODO se tiver não entra
										std::vector<Route> newResult = RouteDefs::copy(solution);
										if ((newResult[i].getTotalDemand() - client.demand + other.demand) < newResult[i].vehicle.capacity &&
											(newResult[j].getTotalDemand() + client.demand - other.demand) < newResult[j].vehicle.capacity) { // Verifica se cabe nos dois
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
	}
	if (resultEval < evaluation) {
		swap11Improve++;
		//auxStruct->improvementChanged(INTERROUTETYPES::SWAP1_1, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::swap2_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		const Route& routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			const Route& routeJ = solution[j];
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
							const Client& client = routeI.clientsList[iClient];
							const Client& next = routeI.clientsList[iClient + 1];
							if (client.demand + sumDeliveryJ - maxDeliveryJ <= routeJCapacity) {
								for (int jClient = 0; jClient < routeJ.clientsList.size(); jClient++) {
									const Client& other = routeJ.clientsList[jClient];
									if (routeI.getTotalDemand() - client.demand - next.demand + other.demand < routeI.vehicle.capacity &&
										routeJ.getTotalDemand() - other.demand + client.demand + next.demand < routeJ.vehicle.capacity) {
										if (routeI.findClient(other.id) < 0 && routeJ.findClient(client.id) < 0 && routeJ.findClient(next.id) < 0) { // TODO se tiver não entra
											std::vector<Route> newResult = RouteDefs::copy(solution);
											RouteDefs::swapClients(newResult[i], next.id, newResult[j], other.id);
											newResult[i].removeClient(client.id);
											newResult[j].insertClient(Client(client), jClient);
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
		}
	}
	if (resultEval < evaluation) {
		swap21Improve++;
		//auxStruct->improvementChanged(INTERROUTETYPES::SWAP1_1, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::swap1_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		const Route& routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			const Route& routeJ = solution[j];
			if (routeI.id != routeJ.id) {
				bool nStatusI = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP1_1, routeI.id);
				bool nStatusJ = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP1_1, routeI.id);
				float minDeliveryI = auxStruct->minDelivery(i);
				float maxDeliveryJ = auxStruct->maxDelivery(j);
				float sumDeliveryJ = auxStruct->sumDelivery(j);
				float routeJCapacity = routeJ.vehicle.capacity;
				if (nStatusI || nStatusJ) {
					int routeISize = routeI.clientsList.size();
					if (routeISize > 0) {
						for (int iClient = 0; iClient < routeISize; iClient++) {
							const Client& client = routeI.clientsList[iClient];
							for (int jClient = 0; jClient < routeJ.clientsList.size(); jClient++) {
								const Client& other = routeJ.clientsList[jClient];
								if (routeI.findClient(other.id) < 0 && routeJ.findClient(client.id) < 0) { // TODO só presseguir se não existe.
									std::vector<Route> newResult = RouteDefs::copy(solution);
									if (client.demand > other.demand) {  // i > j
										if ((newResult[i].getTotalDemand() + other.demand) < newResult[i].vehicle.capacity &&
											(newResult[j].getTotalDemand() + client.demand - other.demand) < newResult[j].vehicle.capacity) { // Verifica se cabe nos dois
											Client copyI(client);
											Client copyJ(other);
											copyI.demand = other.demand;
											newResult[i].clientsList[iClient].demand -= other.demand; // Se não tiver isso fica em excesso
											std::vector<Client> cList;
											cList.push_back(copyJ);
											int indexToInsert = RouteDefs::findBestInsertion(newResult[i], cList, adjCosts, iClient).second;
											if (indexToInsert != -1) { // Só inserir se um indice foi encontrado.
												newResult[i].insertClient(other, indexToInsert);
												newResult[j].clientsList[jClient] = copyI;
												newResult[j].removeClient(other.id);
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
									else if (client.demand < other.demand) {
										if ((newResult[i].getTotalDemand() - client.demand + other.demand) < newResult[i].vehicle.capacity &&
											(newResult[j].getTotalDemand() + client.demand) < newResult[j].vehicle.capacity) { // Verifica se cabe nos dois
											Client copyI(client);
											Client copyJ(other);
											copyJ.demand = client.demand;
											newResult[j].clientsList[jClient].demand -= client.demand; // Se não tiver isso fica em excesso
											std::vector<Client> cList;
											cList.push_back(copyI);
											int indexToInsert = RouteDefs::findBestInsertion(newResult[j], cList, adjCosts, iClient).second;
											if (indexToInsert != -1) { // Só inserir se um indice foi encontrado.
												newResult[j].insertClient(client, indexToInsert);
												newResult[i].clientsList[iClient] = copyJ;
												newResult[i].removeClient(client.id);
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
		}
	}
	if (resultEval < evaluation) {
		swap11SImprove++;
		//auxStruct->improvementChanged(INTERROUTETYPES::SWAP1_1, )
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::swap2_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<Route> result = RouteDefs::copy(solution);
	int resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		const Route& routeI = solution[i];
		for (int j = 0; j < solution.size(); j++) {
			const Route& routeJ = solution[j];
			if (routeI.id != routeJ.id) {
				bool nStatusI = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP1_1, routeI.id);
				bool nStatusJ = auxStruct->neighborhoodStatus(INTERROUTETYPES::SWAP1_1, routeI.id);
				float minDeliveryI = auxStruct->minDelivery(i);
				float maxDeliveryJ = auxStruct->maxDelivery(j);
				float sumDeliveryJ = auxStruct->sumDelivery(j);
				float routeJCapacity = routeJ.vehicle.capacity;
				if (nStatusI || nStatusJ) {
					int routeISize = routeI.clientsList.size();
					if (routeISize > 0) {
						for (int iClient = 0; iClient < routeISize - 1; iClient++) {
							const Client& clientI1 = routeI.clientsList[iClient];
							const Client& clientI2 = routeI.clientsList[iClient + 1];
							for (int jClient = 0; jClient < routeJ.clientsList.size(); jClient++) {
								const Client& other = routeJ.clientsList[jClient];
								if (routeI.getTotalDemand() - clientI1.demand - clientI2.demand + other.demand < routeI.vehicle.capacity &&
									routeJ.getTotalDemand() - other.demand + clientI1.demand + clientI2.demand < routeJ.vehicle.capacity) {
									if (clientI1.demand + clientI2.demand > other.demand && clientI1.demand < other.demand) {  // i > j
										if (routeI.findClient(other.id) < 0 && routeJ.findClient(clientI1.id) < 0 && routeJ.findClient(clientI2.id) < 0) { // TODO só presseguir se não existe.
											std::vector<Route> newResult = RouteDefs::copy(solution);
											Client copyI1(clientI1);// dI
											Client copyI2(clientI2);// dJ
											Client copyJ(other);
											copyI2.demand = other.demand - clientI1.demand;
											newResult[i].clientsList[iClient + 1].demand -= copyI2.demand; // Se não tiver isso fica em excesso
											std::vector<Client> cList;
											cList.push_back(copyJ);
											int indexToInsert = RouteDefs::findBestInsertion(newResult[i], cList, adjCosts, iClient).second;
											if (indexToInsert != -1) { // Só inserir se um indice foi encontrado.
												newResult[j].clientsList[jClient] = copyI2;
												newResult[j].insertClient(copyI1, jClient);
												newResult[i].removeClient(copyI1.id);
												//newResult[i].removeClient(copyI2.id);
												newResult[i].insertClient(copyJ, indexToInsert);
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
									else if (clientI1.demand + clientI2.demand < other.demand){
										
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
		swap21SImprove++;
		return result;
	}
	else {
		return solution;
	}
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
									crossImprove++;
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
		crossImprove++;
		return result;
	}
	else {
		return solution;
	}
}

std::vector<Route> interrouteStructures::routeAddition(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts) {
	std::vector<Route> result = RouteDefs::copy(solution);
	exit(1);
}

std::vector<Route> interrouteStructures::kSplit(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients) {
	std::vector<int> resolvedClients;
	std::vector<Route> result = RouteDefs::copy(solution);
	float resultEval = evaluation;
	for (int i = 0; i < solution.size(); i++) {
		const Route& currRoute = solution[i];
		for (int j = 0; j < currRoute.clientsList.size(); j++) {
			bool hasExplored = false;
			for (int resolvedClientsId : resolvedClients) {
				if (resolvedClientsId == solution[i].clientsList[j].id) {
					hasExplored = true;
					break;
				}
			}
			if (!hasExplored) {
				std::vector<Route> step = RouteDefs::copy(solution);
				Client client = step[i].takeClient(j);
				resolvedClients.push_back(client.id);
				int indx = -1;
				bool foundInMore = (RouteDefs::removeClientFromSolution(step, client.id, indx) > 1);
				int forbiddenIndex = -1;
				if (foundInMore) {
					forbiddenIndex = i;
				}
				bool success = false;
				std::vector<Route> stepResult;
				stepResult = fractionRoute::splitReinsertion(step, RouteDefs::getOriginalClient(client.id, originalClients), forbiddenIndex, success, adjCosts);
				if (success) {
					float eval = RouteDefs::evaluate(stepResult, adjCosts);
					if (eval < resultEval && RouteDefs::isSolutionValid(stepResult, originalClients)) {
						resultEval = eval;
						result = stepResult;
					}
				}
			}
		}
	}
	if (resultEval < evaluation) {
		//auxStruct->improvementChanged(INTERROUTETYPES::SWAP1_1, )
		kSplitImprove++;
		return result;
	}
	else {
		return solution;
	}
	return result;
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
		return swap1_1S(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::SWAP2_1S:
		return swap2_1S(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::ROUTEADDITION:
		return routeAddition(solution, evaluation, auxStruct, adjCosts);
		break;
	case INTERROUTETYPES::KSPLIT:
		return kSplit(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	case INTERROUTETYPES::CROSS:
		return cross(solution, evaluation, auxStruct, adjCosts, originalClients);
		break;
	default:
		cout << "Erro ao escolher interrota.";
		exit(1);
		break;
	}
	RouteDefs::isSolutionValid(solution, originalClients);
}