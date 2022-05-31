#pragma once
#include <vector>
#include <list>

struct ClientAdjacency {
	int clientFromId;
	int clientToId;
	int travelCost;
};

struct Client {
	int id;
	int demand;
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
	int capacity;
	int cost;
	int travelCost;

	bool operator==(const Vehicle& other) {
		return this->id = other.id;
	}

	bool operator!=(const Vehicle& other) {
		return this->id != other.id;
	}
};

