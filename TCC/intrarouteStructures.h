#pragma once

enum class NEIGHBORHOODTYPES {
	EXCHANGE,
	TWO_OPT,
	REINSERTION,
	OR_OP2,
	OR_OP3,	
};

/*
* @brief namespace para execu��o das buscas locais do Iterated Local Search.
*/
namespace intrarouteStructures {

	static void exchange();

	static void two_opt();

	static void reinsertion();

	static void or_op2();

	static void or_op3();

	static int typesCount() { return 5; }

}