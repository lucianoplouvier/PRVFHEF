#include "Utils.h"

// Precisa do -1
int Utils::getRandomInt(int minPossibleValue, int maxPossibleValue) {

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(minPossibleValue, std::nextafter(maxPossibleValue, INT_MAX));
	return dist(mt);
}