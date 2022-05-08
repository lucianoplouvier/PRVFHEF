#include "Utils.h"

int Utils::getRandomInt(int minPossibleValue, int maxPossibleValue) {
	srand(time(NULL));
	return (rand() % maxPossibleValue) + minPossibleValue;
}