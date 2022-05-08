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
};

struct Vehicle {
	int id;
	int type;
	int capacity;
	int cost;
	int travelCost;
};

