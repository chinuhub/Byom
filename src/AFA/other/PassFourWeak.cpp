/* PassFourWeak.cpp


 *
 *  Created on: 09-Sep-2015
 *      Author: jaganmohini
 */
#include "AFAState.h"
#include <boost/foreach.hpp>
#define INB 0
#include <libfaudes.h>
#include "AFAut.h"
std::set<std::set<AFAStatePtr>> AFAState::PassFourPhaseZero(std::map<AFAStatePtr,std::set<std::set<AFAStatePtr>>>& donemap){
	std::set<std::set<AFAStatePtr>> transset;
 	if(mIsAccepted){
		BOOST_ASSERT_MSG(mType==ORLit,"Serious error");
		std::set<AFAStatePtr> thiset;
		thiset.insert(this);
		transset.insert(thiset);
    donemap.insert(std::make_pair(this,transset));
		return transset;
	}else if(donemap.find(this)!=donemap.end()){
   return donemap.find(this)->second;
  }
  else{
		if(mType==AND || mType==OR){

				std::set<std::string> keyset = getTransitionKeys();
				BOOST_ASSERT_MSG(keyset.size()==1 && keyset.find("0")!=keyset.end(),"Some serious error as till this point also transition should only be on 0");
				BOOST_FOREACH(auto t, mTransitions){
					std::set<AFAStatePtr> setstates(t.second.begin(),t.second.end());
					std::set<std::set<AFAStatePtr>> tobeaddedset;
					std::set<AFAStatePtr> empty;
					tobeaddedset.insert(empty);
					for(std::set<AFAStatePtr>::iterator itset = setstates.begin(); itset!=setstates.end(); itset++)
					{
						AFAStatePtr state = *itset;
						std::set<std::set<AFAStatePtr>> retval = state->PassFourPhaseZero(donemap);
						std::set<std::set<AFAStatePtr>> tmptobeaddedset;
						BOOST_FOREACH(auto singlesetSetStates, tobeaddedset){
							for(std::set<std::set<AFAStatePtr>>::iterator it= retval.begin(); it!=retval.end(); it++){
									std::set<AFAStatePtr> curr = *it;
									//ASSERT that every such state is ORLit type only..
		#ifdef SANITYCHKASSN
									BOOST_FOREACH(auto w, curr)
										BOOST_ASSERT_MSG(w->mType==ORLit,"Some serious problem as by now all states must be only ORLit type");
		#endif
									std::set<AFAStatePtr> tmp;
									tmp.insert(singlesetSetStates.begin(),singlesetSetStates.end());
									tmp.insert(curr.begin(),curr.end());
									tmptobeaddedset.insert(tmp);
									}
							}
						tobeaddedset.clear();
						tobeaddedset.insert(tmptobeaddedset.begin(),tmptobeaddedset.end());
					}
					transset.insert(tobeaddedset.begin(),tobeaddedset.end());
				}
				//clear 0 labeled edges from this state and insert transset;
				mTransitions.clear();
				BOOST_FOREACH( auto t, transset){
					BOOST_FOREACH(auto w, t)
							BOOST_ASSERT_MSG(w->mType==ORLit,"Some serious issue");

					SetAFAStatesPtr custset(t.begin(),t.end());
					mTransitions.insert(std::make_pair("0",custset));
			
        }
        donemap.insert(std::make_pair(this,transset));

				return transset;
			}else
			{
				//means this is a non-accepting ORLit state.
				//it must have only one child (as now we dont put self loop at all until it is accepting state
				BOOST_ASSERT_MSG(mTransitions.size()==1,"Some serious error");
				BOOST_ASSERT_MSG(mTransitions.begin()->second.size()==1,"Some serious error as it must have only one element");

				AFAStatePtr nextone = *((mTransitions.begin()->second).begin());
				nextone->PassFourPhaseZero(donemap);
				std::set<AFAStatePtr> emptyone;
				emptyone.insert(this);
				transset.insert(emptyone);
        donemap.insert(std::make_pair(this,transset));
				return transset;
			}
		}

}

//following comparator function is used when both AMAP and HMap must match to make two states into same equivalence
//class.


//struct mymapstatecomparator{
bool mymapstatecomparator::operator() (const AFAStatePtr& one, const AFAStatePtr& two) const{
		if(one->mAMap.hash()==two->mAMap.hash())
			return one->mHMap->hash()<two->mHMap->hash();
		else
			return one->mAMap.hash()<two->mAMap.hash();
}
//};


//Following comparator is used when we want to keep info about an edge on sym from a ORLit node to a ANDOR node.

bool tuplecomparator::operator() (const std::tuple<AFAStatePtr,std::string,AFAStatePtr>& one, const std::tuple<AFAStatePtr,std::string,AFAStatePtr>& two) const
{
		AFAStatePtr srcone = std::get<0>(one);
		std::string symone = std::get<1>(one);
		AFAStatePtr destone = std::get<2>(one);

		AFAStatePtr srctwo = std::get<0>(two);
		std::string symtwo = std::get<1>(two);
		AFAStatePtr desttwo = std::get<2>(two);

		if(srcone==srctwo){
			if(symone==symtwo)
				destone<desttwo;
			else
				symone<symtwo;
		}else
			srcone<srctwo;
}


//Following is an important function as it generates equivalence classes as well as put OR, ORlit states in some containers..
//Also it keeps on filling a set to denote that an ORLit node has a path on sym to an ANDOR node.
std::set<std::tuple<std::string,AFAStatePtr>>  AFAState::PassFourPhaseOne(std::set<AFAStatePtr>& ANDORStates, std::set<AFAStatePtr>& ORLitStates,std::set<std::tuple<AFAStatePtr,std::string,AFAStatePtr>, tuplecomparator>& toANDLink, std::map<AFAStatePtr,	std::set<std::tuple<std::string,AFAStatePtr>>>& assumeinfomap)
{
	std::set<std::tuple<std::string,AFAStatePtr>> assumeinfo;
	if(this->mIsAccepted)
		{
		//keep it in bookkeepting structure
				ORLitStates.insert(this);
		return assumeinfo;
		}
  if(assumeinfomap.find(this)!=assumeinfomap.end())
      return assumeinfomap.find(this)->second;


	if(this->mType==ORLit){
		//means this is a nonaccepting ORLit state.
		//it must have only one  child (as now we dont put self loop at all until it is accepting state
		BOOST_ASSERT_MSG(mTransitions.size()==1,"Some serious error");
		BOOST_ASSERT_MSG(mTransitions.begin()->second.size()==1,"Some serious error as it must have only one element");

		AFAStatePtr nextone = *((mTransitions.begin()->second).begin());
		std::string sym = mTransitions.begin()->first;
		//also record the information when there is an edge from ORLit to AND/OR State so taht later we can
		//saturate this link by combining the affect of 0 from AND/OR State.
		if(nextone->mType!=ORLit){
			toANDLink.insert(std::make_tuple(this,sym,nextone));
		}


		std::set<std::tuple<std::string,AFAStatePtr>> retlink = nextone->PassFourPhaseOne(ANDORStates,ORLitStates,toANDLink,assumeinfomap);
		//for every entry in retlink.. add thistate sym assumestate to toANDLink..
		//ONly if mHMap of this node is unsat..
		BOOST_ASSERT_MSG(mUnsatMemoization.find(*mHMap)!=mUnsatMemoization.end(),"Some issue, look into our invariant");
		bool res = mUnsatMemoization.find(*mHMap)->second;
		if(res)
		{
			BOOST_FOREACH(auto t, retlink){
        //dont add if dest is going to lie in same eq class as this one..
        AFAStatePtr des=std::get<1>(t);
        if(this->mAMap.hash()==des->mAMap.hash() && this->mHMap->hash()==des->mHMap->hash())
          continue;
				toANDLink.insert(std::make_tuple(this,std::get<0>(t),std::get<1>(t)));
					}

//possible that the assume phi instruction resulted in an ORLit node.
			//Moreover it can also come from lcas instruction.
						if(AFAut::mProgram->mRevAssumeLHRHMap.find(mAMap)!=AFAut::mProgram->mRevAssumeLHRHMap.end()){
							//assumeinfo.insert(std::make_tuple(AFAut::mProgram->mRevAssumeLHRHMap.find(mAMap)->second,this));
							//this is also important..In fact we can comment out the above statmenet.
							retlink.insert(std::make_tuple(AFAut::mProgram->mRevAssumeLHRHMap.find(mAMap)->second,this));
						}
		}

		//keep it in bookkeepting structure
		ORLitStates.insert(this);
    assumeinfomap.insert(std::make_pair(this,retlink));

		return retlink;//pass it down further.
	}else{
		//means this is an AND/OR state..
		BOOST_FOREACH(auto t, mTransitions){
			BOOST_ASSERT_MSG(t.first=="0","Some serious error");
			BOOST_FOREACH(auto child, t.second){
				std::set<std::tuple<std::string,AFAStatePtr>> retlink = child->PassFourPhaseOne(ANDORStates,ORLitStates,toANDLink,assumeinfomap);
				assumeinfo.insert(retlink.begin(),retlink.end());
			}
		}
		//check if mAMAp of this corresponds to phi of some assume symbol or not..
		//if yes then add that symbol and this to assumeinfo and return..only if hMap of this node is unsat..
		BOOST_ASSERT_MSG(mUnsatMemoization.find(*mHMap)!=mUnsatMemoization.end(),"Some issue, look into our invariant");
		bool res = mUnsatMemoization.find(*mHMap)->second;
		if(res)
		{
					if(AFAut::mProgram->mRevAssumeLHRHMap.find(mAMap)!=AFAut::mProgram->mRevAssumeLHRHMap.end())
					{
						assumeinfo.insert(std::make_tuple(AFAut::mProgram->mRevAssumeLHRHMap.find(mAMap)->second,this));
					}
		}

		//also add this to ANDORState bookkeeping structure;
		ANDORStates.insert(this);
    assumeinfomap.insert(std::make_pair(this,assumeinfo));
		return assumeinfo;
	}
}


struct fa* AFAState::createRepeat(std::string &in){
	struct fa* aut = fa_make_empty();

	struct autstate* init = add_autstate(aut,1);//make it accepting as well.
	std::set<char> chrset(in.begin(),in.end());
	BOOST_FOREACH(auto ch, chrset)
		add_new_auttrans(init,init,ch,ch);
	aut->initial=init;
	return aut;
}


struct fa* AFAState::createOr(std::string &in){
	struct fa* aut = fa_make_basic(FA_EMPTY);
//it seems that sometime we had problem if order of end and init was changed.. or somethignwas set as accepted in construction
	//need to be more comfortable with working of this fa library.
	struct autstate* end= add_autstate(aut,0);//make it accepting as well.
	struct autstate* init = add_autstate(aut,0);//make it non accepting

	std::set<char> chrset(in.begin(),in.end());
	BOOST_FOREACH(auto ch, chrset)
		add_new_auttrans(init,end,ch,ch);
	aut->initial=init;
	end->accept=1;


	return aut;
}

