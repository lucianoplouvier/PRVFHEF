#include "Utils.h"

// Não precisa do -1
int Utils::getRandomInt(int minPossibleValue, int maxPossibleValue) {
	if (minPossibleValue == 0 && maxPossibleValue == 0) {
		return 0;
	}
	srand(time(NULL));
	return (rand() % maxPossibleValue) + minPossibleValue;
}