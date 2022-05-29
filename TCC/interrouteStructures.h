#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "structures.h"
#include "auxiliaryStructures.h"
#include "route.h"
#include <vector>

namespace interrouteStructures {

	/*
	* @brief Executa shift1_0, o qual passa o cliente de uma rota para outra.
	* @param solution - Solução atual.
	* @param evaluation - avaliação atual.
	* @param auxStruct - Struturas auxiliares.
	* @param adjCosts - custos de adjacência.
	*/
	std::vector<Route> shift1_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients);

	std::vector<Route> shift2_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients);

	std::vector<Route> swap1_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients);

	std::vector<Route> swap2_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients);

	std::vector<Route> swap1_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts);

	std::vector<Route> swap2_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts);

	std::vector<Route> cross(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients);

	std::vector<Route> routeAddition(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts);

	std::vector<Route> kSplit(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts);

	std::list<INTERROUTETYPES> getAll();

	/*
	* @brief Executa o movimento de interrota respectivo.
	* @param type - Tipo de interrota.
	* @param solution - Solução atual.
	* @param evaluation - avaliação atual.
	* @param auxStruct - Struturas auxiliares.
	* @param adjCosts - custos de adjacência.
	*/
	std::vector<Route> executeInterroute(INTERROUTETYPES type, std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients);
}