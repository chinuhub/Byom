/*
 * SCTransSystem.h
 *
 *  Created on: 10-Jun-2015
 *      Author: jaganmohini
 */

#ifndef SCTRANSSYSTEM_SCTRANSSYSTEM_H_
#define SCTRANSSYSTEM_SCTRANSSYSTEM_H_
#include <libfaudes.h>//WHY this ordering was important.. If I put it after c++/z3++ then lots of error..
#include "../Parser/Program.h"
#include <set>
extern "C"
{
#include <fa.h>
}
#include <map>
#include <iostream>
#include <vector>
#include <list>
#include <tuple>
#include <c++/z3++.h>

struct newsetofstatescomparator{
	bool operator() (const struct autstate* one, const struct autstate*  two) const
	{
		return one<two;
	}
};
class SCTransSystem {
	Program& mProgram;
	z3::solver& mSolver;

	std::map<struct autstate*, z3::expr> mShuffautAssnMap;
public:
	struct fa* mMerged;
	SCTransSystem(Program&,z3::solver& s);
	struct fa* BuildSCTS(faudes::Generator&);
	struct fa* FA_Merge(std::vector<struct fa*>& autset,std::map<std::string,z3::expr>& assnMap,faudes::Generator&);
	std::tuple<bool,z3::expr> GetAcceptAssn(std::set<struct autstate*, newsetofstatescomparator>&, std::map<struct autstate*, z3::expr>&);
	std::tuple<std::string,z3::expr> GetAcceptedWordWithEndState();
	z3::expr GetEndStateAssertionFromWord(std::string afaword);
	virtual ~SCTransSystem();//here delete all expr stored in mShuffautAssnMap, fa as well..
};

#endif /* SCTRANSSYSTEM_SCTRANSSYSTEM_H_ */
