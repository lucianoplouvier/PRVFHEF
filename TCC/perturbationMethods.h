#pragma once
#include "structures.h"
#include <vector>
#include "Route.h"
#include "Utils.h"

enum class PERTURBATIONTYPES {
	MULTISWAP,
	MULTISHIFT,
	MERGE,
	KSPLIT
};

namespace perturbationMethods { // Não retorna nada pois ele verifica antes de fazer movimentos inválidos, aí altera diretamente.

	std::vector<PERTURBATIONTYPES> getAll();

	void multipleSwap1_1(std::vector<Route>& solution);

	void multiShift1_1(std::vector<Route>& solution);

	void split(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList);

	/*
	* @brief Junta duas Rotas. A primeira rota é selecionada aleatóriamente. A segunda rota é a rota com o cliente mais próximo a qualquer cliente da primeira rota.
	* Somente pares de rotas que usam um veículo que não seja o maior veículo podem ser escolhidas.
	* @param solution - Solução.
	* @param adjacencyCosts - Custos de adjacencia.
	*/
	void merge(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList);

	void perturbate(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList);

	void executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList);

};