/*
 * AFAut.h
 *
 *  Created on: 23-Aug-2015
 *      Author: jaganmohini
 */

#ifndef AFA_AFAUT_H_
#define AFA_AFAUT_H_
#include "AFAState.h"
#include "../Parser/Program.h"
#include <string>
#include <set>
#include <boost/assert.hpp>
#include <algorithm>
extern "C"
{
#include <fa.h>
}
#include <libfaudes.h>
class AFAut {
private:
	// Stores the initial state of AFA.

	static AFAStatePtr RecMakeAFAutFromFA(struct autstate* state, std::map<struct autstate*, AFAStatePtr>&,z3::context& );
public:
	AFAut(){}
	AFAStatePtr mInit;


	bool IsUnsafe();
	static Program* mProgram;
	/**
	 * Function to convert an NFA/DFA to an AFA
	 */
	static AFAut* MakeAFAutFromFA(struct fa* nfa,Program* program,z3::context&);

	static AFAut* MakeAFAutProof(std::string& word, z3::expr& mPhi,Program* p, int count, bool& bres, faudes::Generator& generator);
	struct fa* ConvertToNFA();
	/**
	 * Complement this AFA and changes init accordingly
	 */
	AFAStatePtr Complement(AFAStatePtr);
	void RecComplement(AFAStatePtr , std::map<AFAStatePtr,bool>& , SetAFAStatesPtr,AFAStatePtr);
	/**
	 * Returns this union second
	 */
	void Union(AFAut& second);

	/**
	 * Returns this intersection secon
	 */
	void Intersection(AFAut& second);
	void Intersection(faudes::Generator& , faudes::Generator& );

	void PrintToDot(std::string filename);
	/**
	 * Extract one accepted word of this AFA and returns that
	 */
	std::string GetWord();

	AFAut* PassFourNew(AFAStatePtr init, std::set<AFAStatePtr>& allStates , int, faudes::Generator&);

	std::set<std::set<AFAStatePtr>> GetConjunctedTransitions(std::set<AFAStatePtr> stateset, std::string sym);
	std::set<AFAStatePtr> GetDisjunctedTransitions(std::set<AFAStatePtr> stateset, std::string sym);


	/**
	* Checks if the given word is accepted by this AFA or not
	*/
	bool IsAccepted(std::string word);

	virtual ~AFAut();
};


#endif /* AFA_AFAUT_H_ */
