#pragma once
#include "structures.h"
#include <vector>
#include "Route.h"
#include "Utils.h"

enum class PERTURBATIONTYPES {
	MULTISWAP,
	MULTISHIFT,
	MERGE,
	KSPLIT,
	SPLIT
};

namespace perturbationMethods { // N�o retorna nada pois ele verifica antes de fazer movimentos inv�lidos, a� altera diretamente.

	std::vector<PERTURBATIONTYPES> getAll(bool hasLimitedVels);

	void multipleSwap1_1(std::vector<Route>& solution);

	void multiShift1_1(std::vector<Route>& solution);

	/*
	* @brief Executa o ksplit aleat�riamente em clientes.
	* @param solution - Solu��o atual.
	* @param adjacencyCosts - Custos de adjac�ncia.
	* @param vehiclesList - Lista de ve�culos.
	* @param clientList - Lista de clientes.
	*/
	void ksplit(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList);

	void split(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, const std::vector<Vehicle>& vehiclesList, RouteCreator& routeCreator, const std::vector<int>& availableVels);

	/*
	* @brief Junta duas Rotas. A primeira rota � selecionada aleat�riamente. A segunda rota � a rota com o cliente mais pr�ximo a qualquer cliente da primeira rota.
	* Somente pares de rotas que usam um ve�culo que n�o seja o maior ve�culo podem ser escolhidas.
	* @param solution - Solu��o.
	* @param adjacencyCosts - Custos de adjacencia.
	*/
	void merge(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels);

	void perturbate(std::vector<Route>& solution, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels);

	void executePerturbation(std::vector<Route>& solution, PERTURBATIONTYPES type, AdjacencyCosts& adjacencyCosts, RouteCreator& creator, const std::vector<Vehicle>& vehiclesList, const std::vector<Client>& clientList, const std::vector<int>& availableVels);

};