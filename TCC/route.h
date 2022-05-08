#pragma once
#include <vector>
#include <list>
#include "structures.h"

struct Route {
	int id;
	std::vector<int> clientsList;
	std::vector<int> demands;
	Vehicle vehicle;

	bool canAddClient(int demand) {
		return demand <= (vehicle.capacity - getTotalDemand());
	}

	bool addClient(int clientId, int demand) {
		bool canAdd = this->canAddClient(demand);
		if (canAdd) {
			clientsList.push_back(clientId);
			demands.push_back(demand);
		}
		return canAdd;
	}

	int getTotalDemand() {
		int totalDemand = 0;
		for (int i = 0; i < (int)demands.size(); i++) {
			totalDemand += demands[i];
		}
		return totalDemand;
	}

	inline bool operator==(const Route& other) {
		return other.id == this->id;
	}
};

class RouteCreator {

public:

	RouteCreator();

	~RouteCreator();

	Route createRoute();

private:

	int m_lastId = 0;


};