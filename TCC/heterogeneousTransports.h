#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <list>
#include <Vector>
#include "structures.h"
#include "interrouteStructures.h"
#include "perturbationMethods.h"
#include "auxiliaryStructures.h"

class PRVFHEF {

public:

	/*
	* @brief Encontra a solução do problema de PRVFHEF.
	* @param clientsDemands - Demanda dos clientes. Cada cliente é representado com um ID que é o seu índice na lista de demandas.
	* @param clientAdjacencies - Lista de adjacencias de clientes com custo.
	* @param vehicleTypes - Lista de tipos de veículos disponíveis.
	* @param vehicles - Quantidade de veículos iniciais.
	*/
	PRVFHEF(std::vector<int> clientsDemands, std::vector<ClientAdjacency> clientAdjacencies, std::vector<Vehicle> vehicleTypes, int vehicles = -1);

	~PRVFHEF();

	/*
	* @brief Avalia uma solução dada.
	* @return valor da avaliação. O Quanto menor, melhor.
	*/
	int evaluate(std::vector<Route> solution) const;

private:

	RouteCreator m_routeCreator;

	AuxiliaryStructures m_auxiliaryStructures;

	std::vector<Client> m_allClients; // Lista de clientes, com id e demanda.

	std::vector<Vehicle> m_vehicleTypes; // Lista de tipos de veículos.

	int m_clientsCount; // Quantidade de clientes.

	std::vector<std::vector<int>> m_adjacencyCosts; // matriz como i * m_clientsCount + j

	/*
	* @brief Cria uma solução inicial.
	* @param vehiclesCount - Quantidade de veículos.
	*/
	std::vector<Route> createInitialSolution(int vehiclesCount);

	/*
	* @brief Inicializa a lista de candidatos.
	* @param allClientes - Lista inicial de todos os clientes.
	* @return Lista de candidados que tem todos os clientes.
	*/
	std::list<int> initializeCandidatesList(const std::vector<Client>& allClients) const;

	/*
	* @brief Recupera o maior veículo disponível.
	* @return Maior veículo na lista de veículos.
	*/
	Vehicle getBiggestVehicle() const;

	/*
	* @brief Faz a estimativa de veículos necessários para atender a demanda de todos os clientes de um problema.
	* @param allClients - Todos os clientes.
	*/
	int estimateVehicles(const std::vector<Client>& allClients) const;

	/*
	* @brief Executa o ILS.
	* @param initialVehicles - quantidade de veículos iniciais.
	* @param iterations - Número máximo de iterações antes de retornar uma resposta final.
	* @param maxItersNoImprove - Número máximo de iterações do ILS sem melhoria.
	*/
	void execute(int initialVehicles, int iterations, int maxItersNoImprove);

	/*
	* @brief Cria a matriz de adjacencias, guardando o resultado em m_adjacencyCosts.
	* @param clientsCount - Quantidade de clientes.
	* @param clientAdjacencies - Adjacencias dos clientes.
	*/
	void createAdjacencyMatrix(int clientsCount, std::vector<ClientAdjacency>& clientAdjacencies);

	/*
	* @brief Executa o RVND.
	* @param currSol - Solução atual.
	*/
	std::vector<Route> rvnd(std::vector<Route>& currSol);

	/*
	* @brief Faz a inserção paralela.
	* @param route - Rotas.
	* @param candidatesList - Lista de candidatos.
	* @param insertionCriteria - Critério de inserção.
	*/
	void paralelInsertion(std::vector<Route>& routes, std::list<int>& candidatesList, bool insertionCriteria);

	/*
	* @brief Recupera o custo de inserção para o cliente mais próximo na rota route do cliente de id informado.
	* @param route - Rota a procurar o menor custo de inserção do cliente mais próximo.
	* @param candidateId - Id do cliente a ser inserido.
	* @return custo.
	*/
	int getClosestInsertionCost(const Route& route, int candidateId) const;

	/*
	* @brief Recupera os custos de adjacência entre dois clientes.
	* @param client1 - Id do cliente 1.
	* @param client2 - Id do cliente 2.
	* @return custo de adjacência entre os dois clientes.
	*/
	int getAdjacencyCosts(int client1, int client2) const;

	//s, i, , rTraço;
	std::vector<Route> splitReinsertion(const std::vector<Route>& solution, int clientIndex, int clientDemand, int forbiddenRouteId);

	std::vector<Route> knaapSackGreedy(const std::vector<Route>& solution, int clientIndex, int clientDemand, 
		std::vector<int>& routesIndexWithResidual, std::vector<int>& residualsList, std::vector<int>& leastInsertionCosts);

};