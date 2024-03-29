#pragma once
#include "route.h"
#include <map>

// Estruturas Auxiliares para a busca interrota.
class AuxiliaryStructures {

public:
	
	/*
	* @brief Construtor.
	* @param adjacencies - Matriz de custos de adjac�ncias de clientes.
	* @param neighborhoodTypes - Quantidade de Tipos de vizinhan�a interrota.
	*/
	AuxiliaryStructures(const AdjacencyCosts* adjacenciesCosts, int neighborhoodTypes);

	/*
	* @brief Estado inicial da rota.
	*/
	~AuxiliaryStructures();

	/*
	* @brief Recalcula os valores das estruturas de dados auxiliares.
	*/
	void recalculate(const std::vector<Route>& routes);

	/*
	* @brief Recupera a soma das demandas a entregar.
	* @param route - Rota de interesse.
	* @return maior custo de entrega da rota.
	*/
	double sumDelivery(int routeIndex) const;

	/*
	* @brief Recupera a menor demanda em uma dada rota.
	* @param route - Rota de interesse.
	* @return menor custo de entrega da rota.
	*/
	double minDelivery(int routeIndex) const;

	/*
	* @brief Recupera a maior demanda em uma dada rota.
	* @param route - Rota de interesse.
	*/
	double maxDelivery(int routeIndex) const;

	/*
	* @brief Recupera a menor demanda entre todos os pares de clientes adjacentes em uma rota.
	* @param route - Rota de interesse.
	* @return Valor da maior demanda entre um par de clientes na rota.
	*/
	double minPairDelivery(int routeIndex) const;

	/*
	* @brief Recupera a maior demanda entre todos os pares de clientes adjacentes em uma rota.
	* @param route - Rota de interesse.
	* @return Valor da maior demanda entre um par de clientes na rota.
	*/
	double maxPairDelivery(int routeIndex) const;

	/*
	* @brief Recupera a demanda cumulativa dos clientes em uma rota.
	* @param routeIndex - Rota de interesse.
	* @param customers - N�mero de clientes a contar a demanda, come�ando do primeiro cliente da rota.
	* @return Valor de demandas acumuladas.
	*/
	double cumulativeDelivery(int routeIndex, int customers) const;

	/*
	* @brief Recupera se a rota informada foi alterada ap�s falhar ao tentar uma melhoria da solu��o.
	* @param neighborhoodtype - N�mero da vizinhan�a.
	* @param routeId - Id Rota de interesse.
	* @return True se a rota foi alterada de alguma forma, false caso contr�rio.
	*/
	bool neighborhoodStatus(INTERROUTETYPES neighborhoodtype, int routeId) const;

	void routeAltered(INTERROUTETYPES neighborhoodType, int routeId, bool alteration);

	void improvementChanged(INTERROUTETYPES neighborhoodType, int routeId, bool improvementChange);

private:

	struct neighborhoodStatusNode {
		bool routeAltered = false;
		bool improvementStatus = true;
	};

	std::vector<double> m_sumDelivery;
	std::vector<double> m_minDelivery;
	std::vector<double> m_maxDelivery;
	std::vector<double> m_minPairDelivery;
	std::vector<double> m_maxPairDelivery;
	std::vector<std::vector<double>> m_cumulativeDelivery;
	std::vector<std::map<int, neighborhoodStatusNode>> m_neighborhoodStatus;

	const AdjacencyCosts* m_adjacencyMatrix;

	int m_neighborhoodTypes;

	void calculateSumDelivery(const std::vector<Route>& routes);
	void calculateMinDelivery(const std::vector<Route>& routes);
	void calculateMaxDelivery(const std::vector<Route>& routes);

	void calculateMinPairDelivery(const std::vector<Route>& routes);
	void calculateMaxPairDelivery(const std::vector<Route>& routes);

	void calculateCumulativeDelivery(const std::vector<Route>& routes);

	// Por enquanto cada rodada de recalculate � falso.
	void calculateNeighborhoodStatus(const std::vector<Route>& routes);

};