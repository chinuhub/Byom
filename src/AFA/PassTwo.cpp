/*
 * PassTwo.cpp

 *
 *  Created on: 25-Aug-2015
 *      Author: jaganmohini
 */


#include "AFAState.h"
#include<boost/foreach.hpp>
#include<unordered_map>



//Changed the return type to show that the mHMap of this node/state is unsat.
bool AFAState::PassTwo(std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& mAllStates){
//	std::cout<<"Inside pass two loop for node with type= "<<mType<<" mamap="<<mAMap<<" and hmap= "<<mHMap<<std::endl;
	//check if we have already seen this state.. if yes then return
	if(mAllStates.find(this)!=mAllStates.end()){
//		std::cout<<"found and returne d"<<std::endl;
		BOOST_ASSERT_MSG(mUnsatMemoization.find(*mHMap)!=mUnsatMemoization.end(),"Some serious issue");
		return mUnsatMemoization.find(*mHMap)->second;
	}
	else{
		mAllStates.insert(std::make_pair(this,this));
//		std::cout<<" not found and added "<<std::endl;
	}

	if(mType==AND){
		//collect the assertion AHMap on every outgoing state of this stte..
//		std::cout<<"Transition size is "<<mTransitions.size()<<std::endl;
//		BOOST_ASSERT_MSG(mTransitions.size()==1," Some problem as the size of transitions must be 1 initially");
		//RATHER than above assertion, (Because now a self loop gets added in construction) check if getKeyset of
		//this state is singleton (only 0) or not..
#ifdef	DBGPRNT
		std::set<std::string> keyset = this->getTransitionKeys();
		BOOST_ASSERT_MSG(keyset.size()==1 && (keyset.find("0")!=keyset.end())," Some problem as this state must have only outgoing edges on 0");
#endif
		z3::context& ctx = mAMap.ctx();
		z3::solver solv(ctx);
		z3::params pc(ctx);
		pc.set(":unsat-core",true);
		solv.set(pc);
		/////////////This is the place We should put repeat at.. so that we can search for all possible
		//////////////unsat cores..
		SetAFAStatesPtr newset;
		SetAFAStatesPtr nextset;
		BOOST_FOREACH(auto v, mTransitions){
			if(v.second.find(this)!=v.second.end())//means it denotes self loop then dont do anything.
			{}
			else
				nextset=v.second;
		}
#ifdef	DBGPRNT
		BOOST_ASSERT_MSG(nextset.size()!=1,"Some serious error in our understanding look into it");
#endif
		//MAYBE here it doesn not matter as we have already checked that the size must be one and that too with 0.
		bool unchanged=false;
		std::multimap<std::string,SetAFAStatesPtr> temptransitions;
		while(unchanged!=true)//this to ensure that we find all possible unsat cores after removing them individually.
		{
			unchanged=true;
			SetAFAStatesPtr result;


			std::set_difference(nextset.begin(),nextset.end(),newset.begin(),newset.end(),std::inserter(result,result.end()),nextset.value_comp());

			nextset.clear();
			nextset.insert(result.begin(),result.end());
			newset.clear();

			int i=0;
			std::map<z3::expr,AFAStatePtr,mapexpcomparator> labstatemapping;
			std::vector<z3::expr> assumptions;
			solv.reset();
			BOOST_FOREACH(auto stp, nextset)
			{
				std::string s = std::to_string(i);
				z3::expr sexp = ctx.bool_const(s.c_str());
				//construct a formula.. in solver so that we can give it to satsolver to check satisfiability
				z3::expr formula = *((*stp).mHMap);
				solv.add(implies(sexp,formula));
				assumptions.push_back(sexp);
				labstatemapping.insert(std::make_pair(sexp,stp));
				i++;
			}

			if(solv.check(assumptions.size(),&assumptions[0])==z3::check_result::unsat){
				unchanged=false;
				z3::expr_vector core = solv.unsat_core();
				for (unsigned i = 0; i < core.size(); i++) {
					z3::expr nm = core[i];
#ifdef	DBGPRNT
					BOOST_ASSERT_MSG(labstatemapping.find(nm)!=labstatemapping.end(),"Some problem this should not have happened");
#endif
					newset.insert(labstatemapping.find(nm)->second);
			    	}
				temptransitions.insert(std::make_pair("0",newset));

			}
			//break;
		}//end of loop
		bool res;
		if(temptransitions.size()!=0){
			//means it is unsat..
			res=true;
			mUnsatMemoization.insert(std::make_pair(*mHMap,true));
			mTransitions.clear();
			std::copy(temptransitions.begin(),temptransitions.end(),std::inserter(mTransitions,mTransitions.begin()));
		}else
		{
			res=false;
			mUnsatMemoization.insert(std::make_pair(*mHMap,false));
		}
		BOOST_FOREACH(auto st, mTransitions){
					BOOST_FOREACH(auto st2, st.second)
							st2->PassTwo(mAllStates);
				}
		return res;
	}else if(mType==OR){
		//collect the assertion AHMap on every outgoing state of this stte..
#ifdef	DBGPRNT
		std::set<std::string> keyset = this->getTransitionKeys();
		BOOST_ASSERT_MSG(keyset.size()==1 && (keyset.find("0")!=keyset.end())," Some problem as this state must have only outgoing edges on 0");
#endif
		bool res=true;
		BOOST_FOREACH(auto st, mTransitions){
				BOOST_FOREACH(auto st2, st.second)
						res=res && (st2->PassTwo(mAllStates));
				}
		mUnsatMemoization.insert(std::make_pair(*mHMap,res));

		return res;//if both are unsat only then this is unsat..
	}else if(mType==ORLit){
		if(mIsAccepted)
		{//no children for accepting node..
			if(HelperIsUnsat(*mHMap)){
				mUnsatMemoization.insert(std::make_pair(*mHMap,true));
						return true;//true means it is unsat..
			}
			else{
				mUnsatMemoization.insert(std::make_pair(*mHMap,false));
				return false;
			}

		}else
		{
			bool res=false;
			BOOST_ASSERT_MSG(mTransitions.size()==1,"Can not have more than one outgoing edges, some issue");
			BOOST_FOREACH(auto st, mTransitions){
				BOOST_ASSERT_MSG(st.second.size()==1,"Can not  have size >1, some serious issue");
						BOOST_FOREACH(auto st2, st.second)
								res=st2->PassTwo(mAllStates);
					}
			mUnsatMemoization.insert(std::make_pair(*mHMap,res));
			return res;
		}

	}
}

