/*
 * SCTransSystem.cpp
 *
 *  Created on: 10-Jun-2015
 *      Author: jaganmohini
 */

#include "SCTransSystem.h"
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
extern "C" {
#include <stdio.h>
}

typedef std::tuple<struct autstate* ,std::tuple<std::set<struct autstate*,newsetofstatescomparator>,bool>> newstateinfo;

struct newstateinfocompare{
	bool operator() (const newstateinfo* one, const newstateinfo* two) const
		{
		bool first = std::get<1>(std::get<1>(*one));
		bool second = std::get<1>(std::get<1>(*two));
		std::set<struct autstate*,newsetofstatescomparator> firstset = std::get<0>(std::get<1>(*one));
		std::set<struct autstate*,newsetofstatescomparator> secondset = std::get<0>(std::get<1>(*two));
		if(first!=second)
			return first<second;
		else
			return firstset<secondset;
//			if(std::get<1>(std::get<1>(*one))!=std::get<1>(std::get<1>(*two)))
//			return (std::get<1>(*one))<(std::get<1>(*two));
		}
};


 SCTransSystem::SCTransSystem(Program& P, z3::solver& s):mProgram(P), mSolver(s) {
}

 /*
  * Unused function: Delete it later
  */
std::tuple<std::string,z3::expr> SCTransSystem::GetAcceptedWordWithEndState()
{
	struct autstate* accstate=NULL;
	char* word;
	size_t length;
	fa_example_withendstate(mMerged,&word,&length,&accstate);
	std::string accword(word,length);
	std::cout<<"word is "<<accword<<" with length = "<< length<<std::endl;
	BOOST_ASSERT_MSG(mShuffautAssnMap.find(accstate)!=mShuffautAssnMap.end(),"SHould not be possible as this accepted state must have been set properly, check out");
	z3::expr ex = mShuffautAssnMap.find(accstate)->second;
	return (std::tuple<std::string,z3::expr>(accword,ex));
}

z3::expr SCTransSystem::GetEndStateAssertionFromWord(std::string afaword)
{
	size_t length=afaword.length();
	std::string lastsym(afaword.substr(length-1,1));
	std::map<std::string,z3::expr> assnMap = mProgram.GetAssnMapForAllProcesses();
	BOOST_ASSERT_MSG(assnMap.find(lastsym)!=assnMap.end(),"Some serious issue as the last char must be the one where assn is defined");
	return assnMap.find(lastsym)->second;

}

struct fa* SCTransSystem::BuildSCTS(faudes::Generator& lGenerator){
	std::vector<std::string> regexes = mProgram.GetRegexOfAllProcesses();
		struct fa* aut = NULL;
	std::map<std::string,z3::expr> assnMap = mProgram.GetAssnMapForAllProcesses();
		std::vector<struct fa*> automata;

		BOOST_FOREACH(std::string regex, regexes)
		{
#ifdef	DBGPRNT
			std::cout<<"Extracted Regex is "<<regex<<std::endl;
#endif
			//construct an automaton

			int res = fa_compile(regex.c_str(),(size_t)regex.length(),&aut);
			BOOST_ASSERT_MSG(aut!=NULL,"could not construct the automaton from a given expression");
			automata.push_back(aut);

		}

		mMerged = FA_Merge(automata,assnMap,lGenerator);

#ifdef DBGPRNT
		std::cout<<"Is deterministic "<<mMerged->deterministic<<std::endl;
#endif
		/*FILE* OUT;
					OUT=fopen("try.dot","w");
					BOOST_ASSERT_MSG(OUT!=NULL,"Error");
					fa_dot(OUT,mMerged);
					fclose(OUT);*/

		struct fa* forreverse = fa_clone(mMerged);
		reverseInPlace(forreverse);
//		mMerged->deterministic=0;
//		mMerged->minimal=0;
//		fa_minimize(mMerged);//For some examples generated FA is Non deterministic hence we need to make it deterministic
				//minimiation make it deterministic but we can also separately call determinize which is not exported from fa
				//library yet.. but I am doing this shortcut for checking the correctness .. speed can be improved later.
		return forreverse ;//return reversed of merged FA..

}

struct fa* SCTransSystem::FA_Merge(std::vector<struct fa*>& autset,std::map<std::string,z3::expr>& assnMap, faudes::Generator& lGenerator)
{
	std::set<struct autstate*, newsetofstatescomparator> initset;
	std::map<struct autstate*, z3::expr> stateAssnMap;

	BOOST_FOREACH(auto elem, autset){
		initset.insert(elem->initial);
	}
//add all symbols to generator

	BOOST_FOREACH(auto t, mProgram.mAllSyms){
		//add events
		lGenerator.InsEvent(t);
	}
	//std::cout<<"Size of initset is "<<initset.size()<<std::endl;
	std::map<newstateinfo*, newstateinfo*, newstateinfocompare> seenmap;
		struct fa* merged = fa_make_empty();
		//create a new state, init
		struct autstate* init = add_autstate(merged,0);//add a non-accepting state to this automaton
		std::stringstream st;
		st<<init;
		lGenerator.InsState(st.str());
//		lGenerator.SetInitState(st.str());
    lGenerator.SetMarkedState(st.str());

		//set init as initial state of merged
		merged->initial=init;
		//now for each character in mProgram.mInitString create sequential transitions from init..by adding new states.
		//and transitions .. the last state will be added to newstatinfo object create later..
		struct autstate* last = init;
		std::string laststr(st.str());
		st.str("");
		for(std::string::iterator it = mProgram.mInitString.begin();it!=mProgram.mInitString.end();it++)
		{
			char sym = *it;
			std::string symstr(1,sym);
			struct autstate* curr = add_autstate(merged,0);
			st<<curr;
			lGenerator.InsState(st.str());
			add_new_auttrans(last,curr,sym,sym);
			lGenerator.SetTransition(st.str(),symstr,laststr);
			last=curr;
			laststr=std::string(st.str());
			st.str("");
		}

		//map last to initset
		newstateinfo* initinfo = new newstateinfo;
		std::get<0>(*initinfo) = last;
		std::get<0>(std::get<1>(*initinfo)) = initset;
		std::get<1>(std::get<1>(*initinfo)) = false;
		//initset must be non-accepting (some weak link here) so set bool as false.
		seenmap[initinfo]=initinfo;
		//put init to a worklist, arraylist of struct autstate*
				std::list<newstateinfo*> worklist;
				worklist.push_back(initinfo);
				while(worklist.size()!=0)
				{
					newstateinfo* picked = worklist.front();
					worklist.pop_front();
					//get state of shuffled automaton.
					struct autstate* shuffstate = std::get<0>(*picked);
					st.str("");
					st<<shuffstate;
					std::string currstring(st.str());
					st.str("");
					std::set<struct autstate*, newsetofstatescomparator> productset = std::get<0>(std::get<1>(*picked));
					//no need to extract bool here..
					BOOST_FOREACH(struct autstate* procstate, productset)
					{
						for(int i=0; i< procstate->tused; i++)
						{
							struct auttrans trans = procstate->trans[i];
							//find the destination state
							struct autstate* dest = trans.to;
							//construct new reachable product states set
							std::set<struct autstate*, newsetofstatescomparator> newproductset = productset;
							newproductset.erase(procstate);
							newproductset.insert(dest);
							//check if the new product set is in seenmap
							newstateinfo* newstepstateinfo = new newstateinfo;
							//before that check if this transition was assertion causing one and hence this state becomes accepting state as well.
							std::string searched(1,trans.min);
							bool isacc=false;
							if(assnMap.find(searched)!=assnMap.end()){
								//means set this as accepting
								isacc=true;
								std::get<1>(std::get<1>(*newstepstateinfo))=true;
							}else
								std::get<1>(std::get<1>(*newstepstateinfo))=false;

							std::get<0>(std::get<1>(*newstepstateinfo))=newproductset;
							struct autstate* newshuffledstate = NULL;
							if(seenmap.find(newstepstateinfo)!=seenmap.end()){
								//means the key was found..and accept/nonaccept was also matched
								//std::cout<<"No new element is added "<<std::endl;

								//get the associated shuffled state.
								newstateinfo* tempnewstepstateinfo = seenmap[newstepstateinfo];
								delete newstepstateinfo;//because now this if of no use..
								newstepstateinfo = tempnewstepstateinfo;
								newshuffledstate = std::get<0>(*newstepstateinfo);
								if(assnMap.find(searched)!=assnMap.end()){
									BOOST_ASSERT_MSG(newshuffledstate->accept==1,"THis must have been set earlier otherwise serious error");
								}
							}else
							{
								//std::cout<<"New element is added "<<std::endl;
								//else newstepstateinfo was correctly allocated..
								//create a new state in shuffled automaton
								newshuffledstate = add_autstate(merged,0);
								st.str("");
								st<<newshuffledstate;
								std::string  deststring(st.str());
								st.str("");
								lGenerator.InsState(deststring);
								if(assnMap.find(searched)!=assnMap.end()){//check if the transition caused by this symbol by this procstate causes it to reach to assertion point
									//means key found
									z3::expr ex = assnMap.find(searched)->second;
									newshuffledstate->accept=1;
									mShuffautAssnMap.insert(std::make_pair(newshuffledstate,ex));
								}else
									newshuffledstate->accept=0;
								std::get<0>(*newstepstateinfo) = newshuffledstate;
								//insert it to seenmap as well as in worklist
								seenmap[newstepstateinfo]=newstepstateinfo;
								worklist.push_back(newstepstateinfo);
							}
							st.str("");
							st<<newshuffledstate;
							std::string  deststring(st.str());
							st.str("");
							if(isacc)
								lGenerator.SetInitState(deststring);//because we are creating reversed generator..
							//add a transition from shuffstate to this shuffled state with label extracted from this transition
							add_new_auttrans(shuffstate, newshuffledstate,trans.min,trans.max);
							lGenerator.SetTransition(deststring,searched,currstring);
						}
					}
				}
				//after we are done.. call delete on every key element of the map seenmap
				//no need to delete value as key is same as value.
				BOOST_FOREACH(auto d, seenmap)
				{
					delete d.first;
				}
#ifdef DBGPRNT
				//minimize automaton and then return it.
				FILE *fp;
				fp = fopen("./originalMerged.dot", "w");
				BOOST_ASSERT_MSG(fp!=NULL,"Can't open input file in.list!\n");

				//no need to determinize here as by construction it is determinzed;
				fa_dot(fp,merged);
				fclose(fp);
#endif

	return merged;
}


std::tuple<bool,z3::expr> SCTransSystem::GetAcceptAssn(std::set<struct autstate*, newsetofstatescomparator>&  productset, std::map<struct autstate*, z3::expr>& stateAssnMap)
{
	bool found=false;
	z3::context& ctx = mSolver.ctx();
	z3::expr trueexp = ctx.bool_val(true);
	BOOST_FOREACH(struct autstate* st, productset)
		{
			if(stateAssnMap.find(st)!=stateAssnMap.end())
			{
				found=true;
				z3::expr ex = stateAssnMap.find(st)->second;
				trueexp = trueexp && ex;
			}
		}
	if(found==false)
		return std::tuple<bool,z3::expr>(false,trueexp);
	else
		return std::tuple<bool,z3::expr>(true,trueexp.simplify());
}

SCTransSystem::~SCTransSystem() {


}

