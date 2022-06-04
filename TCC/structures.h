#pragma once
#include <vector>
#include <list>

struct ClientAdjacency {
	int clientFromId;
	int clientToId;
	float travelCost;
};

struct Client {
	int id;
	float demand;
	Client() {
		id = -1;
		demand = -1;
	}
	Client(const Client& other) {
		this->id = other.id;
		this->demand = other.demand;
	}
};

struct Vehicle {
	int id;
	int type;
	float capacity;
	float cost;
	float travelCost; // Como multiplica, tem que ser pelo menos 1

	bool operator==(const Vehicle& other) {
		return this->id = other.id;
	}

	bool operator!=(const Vehicle& other) {
		return this->id != other.id;
	}
};

