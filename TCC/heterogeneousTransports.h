#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <list>
#include <Vector>
#include "structures.h"
#include "interrouteStructures.h"
#include "intrarouteStructures.h"
#include "perturbationMethods.h"
#include "auxiliaryStructures.h"
#include "Route.h"

using namespace RouteDefs;

class PRVFHEF {

public:

	/*
	* @brief Encontra a solu��o do problema de PRVFHEF.
	* @param clientsDemands - Demanda dos clientes. Cada cliente � representado com um ID que � o seu �ndice na lista de demandas.
	* @param clientAdjacencies - Lista de adjacencias de clientes com custo.
	* @param vehicleTypes - Lista de tipos de ve�culos dispon�veis.
	* @param vehicles - Quantidade de ve�culos iniciais.
	*/
	PRVFHEF(std::vector<float> clientsDemands, std::vector<ClientAdjacency> clientAdjacencies, std::vector<Vehicle> vehicleTypes, std::vector<float> depotTravelCost, int vehicles = -1);

	~PRVFHEF();

	/*
	* @brief Avalia uma solu��o dada.
	* @return valor da avalia��o. O Quanto menor, melhor.
	*/
	float evaluate(std::vector<Route>& solution) const;

private:

	int m_maxVehicles; // Kmin

	RouteCreator m_routeCreator;

	AuxiliaryStructures* m_auxiliaryStructures;

	std::vector<float> m_clientsOriginalDemands;

	std::vector<Client> m_allClients; // Lista de clientes, com id e demanda.

	std::vector<Vehicle> m_vehicleTypes; // Lista de tipos de ve�culos.

	int m_clientsCount; // Quantidade de clientes.

	AdjacencyCosts m_adjacencyCosts; // matriz como i * m_clientsCount + j

	/*
	* @brief Cria uma solu��o inicial.
	* @param vehiclesCount - Quantidade de ve�culos.
	*/
	std::vector<Route> createInitialSolution(int vehiclesCount);

	/*
	* @brief Inicializa a lista de candidatos.
	* @param allClientes - Lista inicial de todos os clientes.
	* @return Lista de candidados que tem todos os clientes.
	*/
	std::list<int> initializeCandidatesList(const std::vector<Client>& allClients) const;

	/*
	* @brief Faz a estimativa de ve�culos necess�rios para atender a demanda de todos os clientes de um problema.
	* @param allClients - Todos os clientes.
	*/
	int estimateVehicles(const std::vector<Client>& allClients) const;

	void addClientToRoute(Route& r, int candidateId, float demandAmount);

	/*
	* @brief Executa o ILS.
	* @param initialVehicles - quantidade de ve�culos iniciais.
	* @param iterations - N�mero m�ximo de itera��es antes de retornar uma resposta final.
	* @param maxItersNoImprove - N�mero m�ximo de itera��es do ILS sem melhoria.
	*/
	void execute(int initialVehicles, int iterations, int maxItersNoImprove);

	/*
	* @brief Cria a matriz de adjacencias, guardando o resultado em m_adjacencyCosts.
	* @param clientsCount - Quantidade de clientes.
	* @param clientAdjacencies - Adjacencias dos clientes.
	* @param depotTravelCost - Custo de viagem entre o dep�sito e o cliente de indice x.
	*/
	void createAdjacencyMatrix(int clientsCount, std::vector<ClientAdjacency>& clientAdjacencies, std::vector<float> depotTravelCost);

	/*
	* @brief Executa o RVND.
	* @param currSol - Solu��o atual.
	* @parm evaluation - Avalia��o atual.
	*/
	std::vector<Route> rvnd(std::vector<Route>& currSol, float evaluation);

	/*
	* @brief Faz a inser��o paralela.
	* @param route - Rotas.
	* @param candidatesList - Lista de candidatos.
	* @param insertionCriteria - Crit�rio de inser��o.
	* @return rotas preenchidas.
	*/
	std::vector<Route> paralelInsertion(std::vector<Route>& routes, std::list<int>& candidatesList, bool insertionCriteria);

	/*
	* @brief Recupera o custo de inser��o para o cliente mais pr�ximo na rota route do cliente de id informado.
	* @param route - Rota a procurar o menor custo de inser��o do cliente mais pr�ximo.
	* @param candidateId - Id do cliente a ser inserido.
	* @return custo.
	*/
	float getClosestInsertionCost(const Route& route, int candidateId) const;

	std::vector<Route> intraroute(const std::vector<Route>& solution, float evaluation);

	void printSolution(float eval, const std::vector<Route>& solution, ofstream& stream);

	bool verifySolutionValid(const std::vector<Route>& solution);

	/*
	* @brief Verifica se � possivel adicionar um candidado da lista � rota.
	*/
	bool canAddAnyToARoute(const std::vector<Route>& routes, const std::list<int>& candidatesList);

	/*
	* @brief Redimensiona o Ve�culo de forma a tentar trocar o ve�culo de uma rota por outro mais econ�mico.
	* @param routes - Rotas.
	* @param currEval - Avalia��o atual
	* @return avalia��o final.
	*/
	float vehicleRedimension(std::vector<Route>& routes, float currEval);

	int m_currIteration; // Come�a do 1.
	int m_currIterationsWithoutImprove;

};