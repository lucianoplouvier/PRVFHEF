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

	bool operator==(Client& other) {
		return this->id == other.id && (std::abs(std::abs(this->demand) - std::abs(other.demand)) < std::numeric_limits<float>::epsilon());
	}

};

struct Vehicle {
	int id;
	float capacity;
	float cost;
	float travelCost; // Como multiplica, tem que ser pelo menos 1
	bool ficticous = false;

	bool operator==(const Vehicle& other) {
		return this->id = other.id;
	}

	bool operator!=(const Vehicle& other) {
		return this->id != other.id;
	}
};

