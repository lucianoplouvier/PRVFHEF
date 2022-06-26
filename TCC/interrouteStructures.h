#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "structures.h"
#include "auxiliaryStructures.h"
#include "route.h"
#include <vector>
#include "fraction.h"

namespace interrouteStructures {

	static int shift10Improve = 0;
	static int shift20Improve = 0;
	static int swap11Improve = 0;
	static int swap21Improve = 0;
	static int swap11SImprove = 0;
	static int swap21SImprove = 0;
	static int crossImprove = 0;
	static int kSplitImprove = 0;

	int getInterrouteSumImprove();

	int getShift10Improve();

	int getShift20Improve();

	int getSwap11Improve();

	int getSwap21Improve();

	int getSwap11SImprove();

	int getSwap21SImprove();

	int getCrossImprove();

	int getKSplitImprove();

	/*
	* @brief Executa shift1_0, o qual passa o cliente de uma rota para outra.
	* @param solution - Solução atual.
	* @param evaluation - avaliação atual.
	* @param auxStruct - Struturas auxiliares.
	* @param adjCosts - custos de adjacência.
	*/
	std::vector<Route> shift1_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> shift2_0(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> swap1_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> swap2_1(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> swap1_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> swap2_1S(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> cross(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::vector<Route> kSplit(std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);

	std::list<INTERROUTETYPES> getAll();

	/*
	* @brief Executa o movimento de interrota respectivo.
	* @param type - Tipo de interrota.
	* @param solution - Solução atual.
	* @param evaluation - avaliação atual.
	* @param auxStruct - Struturas auxiliares.
	* @param adjCosts - custos de adjacência.
	*/
	std::vector<Route> executeInterroute(INTERROUTETYPES type, std::vector<Route>& solution, float evaluation, AuxiliaryStructures* auxStruct, AdjacencyCosts& adjCosts, const std::vector<Client>& originalClients, std::vector<int>& availableVels);
}