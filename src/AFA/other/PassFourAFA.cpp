/*
 * PassFourAFA.cpp

 *
 *  Created on: 23-Sep-2015
 *      Author: jaganmohini
 */

#include "AFAut.h"


AFAut* AFAut::PassFourNew(AFAStatePtr init, std::set<AFAStatePtr>& tobedeleted, int cases,faudes::Generator& lGenerator)
{


	int i;
	//keep all values present in statemap to be deleted after next loop.
	//Pass 1:
	std::set<std::tuple<AFAStatePtr,std::string,AFAStatePtr>, tuplecomparator> toANDLink;
	std::set<AFAStatePtr> ANDORStates;
	std::set<AFAStatePtr> ORLitStates;
  std::map<AFAStatePtr,	std::set<std::tuple<std::string,AFAStatePtr>>> assumeinfomap;
#ifdef DBGPRNT
	std::cout<<"About to construct phase1 pass4"<<std::endl;
#endif

	init->PassFourPhaseOne(ANDORStates,ORLitStates,toANDLink,assumeinfomap);
	//when done in seenmap we have Eq classes for every distinct amap.
	//delete all states present in the set allstates.., No we cant as we are currently in a state only..
	//so its better to return it..
	/*BOOST_FOREACH(auto t, allStates){
		delete(t.first);
	}*/
	//NO change in graph by this pass.. only collecting information to be used in phase 2
	AFAut* tm=new AFAut();
	tm->mInit=init;
//#ifdef DBGPRNT
	//tm->PrintToDot("Pass4Phase1.dot");
	std::cout<<"Pass 1 over"<<std::endl;
	//std::cin>>i;
//#endif
/*
	tm->PrintToDot("Pass4Phase1.dot");
		std::cout<<"Pass 1 over"<<std::endl;
*/
	//Pass0: propoage 0's on OR/AND states so that after this every 0's transition from OR/AND goest to ORLit state only.
	//we can use the fact that by this time we have a nice tree like structure of FA
  std::map<AFAStatePtr,std::set<std::set<AFAStatePtr>>> donemap;
  //std::cout<<"About to construct phase0 pass4"<<std::endl;
	std::set<std::set<AFAStatePtr>> retset = init->PassFourPhaseZero(donemap);
	//if init was OR/AND then all this set will be on 0,
	//if init was ORLit then it will be single state (init) in the returned set


		tm->mInit=init;
//#ifdef DBGPRNT
		//tm->PrintToDot("Pass4Phase0.dot");
		std::cout<<"Pass 0 over"<<std::endl;
		//std::cin>>i;
//#endif


		tobedeleted.insert(ANDORStates.begin(),ANDORStates.end());
		tobedeleted.insert(ORLitStates.begin(),ORLitStates.end());


	//Pass 2: //now we will add transitions  toANDLink,

	//for cases when orlit has a sym labeled edge to OR/AND replace that OR/AND by its transitions which will certainly be OR/AND
	//create epsilon closure for 0 labeled edges coming out from ANDOR eq states..
	//sufficient to look at ORLIt states and check if they have an edge to ANDORStates.. if yes on some symbol say sym
	//then add that ANDOR state's transitions on sym to this state.after remvoing zero to that ANDORState..

	BOOST_FOREACH(auto t, toANDLink){
		AFAStatePtr src = std::get<0>(t);
		std::string sym = std::get<1>(t);
		AFAStatePtr dest = std::get<2>(t);
	 // BOOST_ASSERT_MSG(dest->mType!=ORLit,"Some serious error");
		std::set<std::pair<std::string,std::set<AFAStatePtr>>,transitionscomparatorraw> transsetthisone;
		//asssert AFAut* PassFourNew(AFAStatePtr init, std::set<AFAStatePtr>& allStates , int, faudes::Generator&);
		for(std::multimap<std::string,SetAFAStatesPtr>::iterator it= src->mTransitions.equal_range(sym).first;it!=src->mTransitions.equal_range(sym).second;it++)
		{
			if(it->second.find(dest)!=it->second.end()){
				BOOST_ASSERT_MSG(it->second.size()==1,"Seriour error");
			}else
			{
				std::set<AFAStatePtr> custset(it->second.begin(),it->second.end());
				transsetthisone.insert(std::make_pair(it->first,custset));
			}
		}
		src->mTransitions.erase(sym);
		BOOST_FOREACH(auto t, src->mTransitions){
			std::set<AFAStatePtr> own(t.second.begin(),t.second.end());
			transsetthisone.insert(std::make_pair(t.first,own));
		}

    if(dest->mType!=ORLit)
    {
  		BOOST_FOREACH(auto t, dest->mTransitions){
	  		//BOOST_ASSERT_MSG(t.first=="0","SOmer serious error");
		  	std::set<AFAStatePtr> custset(t.second.begin(),t.second.end());
        //if t.second is singleton and is same as src then dont add...
        if(t.second.size()==1)
        {
          AFAStatePtr ss= *(t.second.begin());
          if(src->mAMap.hash()==ss->mAMap.hash() && src->mHMap->hash()==ss->mHMap->hash())
            continue;
        }
    		transsetthisone.insert(std::make_pair(sym,custset));
		  }
    }else
    {
      std::set<AFAStatePtr> dset;
      dset.insert(dest);
      transsetthisone.insert(std::make_pair(sym,dset));
    }
  	src->mTransitions.clear();
		BOOST_FOREACH(auto t, transsetthisone){
			SetAFAStatesPtr custset(t.second.begin(),t.second.end());
			src->mTransitions.insert(std::make_pair(t.first,custset));
		}
	//	src->mTransitions.insert(transsetthisone.begin(),transsetthisone.end());
	}
	//IMportant is this ordering.. first add all edges which come by virtue of connecting to future actions assume
	//and if any orlit is left without any edge on some assume symbol then add that assume symbol as well..
	//note that in all this addition of edge labelling assume symbol do it only when its hmap is false.
/*
	BOOST_FOREACH(auto orlitstate, ORLitStates){
				bool isorlitunsat = HelperIsUnsat(*(orlitstate->mHMap));
				if(isorlitunsat)
				{
					//add self loops on assume symbols, [ONLY on those on which it does not have already an edge]
					SetAFAStatesPtr own;
					own.insert(orlitstate);
					BOOST_FOREACH(auto sym, AFAut::mProgram->mAssumeLHRHMap){
						if(orlitstate->mTransitions.find(sym.first)==orlitstate->mTransitions.end())
							orlitstate->mTransitions.insert(std::make_pair(sym.first,own));
					}
				}
			}
*/


#ifdef DBGPRNT
		std::cout<<"Pass 2 over"<<std::endl;
		tm->PrintToDot("Pass4Phase2.dot");
		std::cin>>i;
#endif
		std::cout<<"Pass 2 over"<<std::endl;
	//		tm->PrintToDot("Pass4Phase2.dot");
		//	std::cin>>i;



/**********Now create equivalence classes for similar/same amap same hmap states..
 * By this time no AND OR node left.., in the newly create equivalence class add same amap hmap as for all..
 */
std::set<AFAStatePtr> newORLitSet;
			AFAStatePtr newinit;
{//start block


	std::map<AFAStatePtr, AFAStatePtr, mymapstatecomparator> eqclass;
	std::set<AFAStatePtr> seenset;
	std::set<AFAStatePtr> workset;
	BOOST_FOREACH(auto t, ORLitStates){
		AFAStatePtr currentst = t;
		AFAStatePtr newst;
		if(eqclass.find(currentst)==eqclass.end()){
			newst = new AFAState(ORLit,currentst->mAMap);
			//tobedeleted.insert(newst);
			newst->mHMap=new z3::expr(*(currentst->mHMap));
			eqclass.insert(std::make_pair(currentst,newst));
		}else{
			newst=eqclass.find(currentst)->second;
		}
		newORLitSet.insert(newst);
		if(t->mIsAccepted)
			newst->mIsAccepted=true;
		std::set<std::pair<std::string,std::set<AFAStatePtr>>,transitionscomparatorraw> transsetthisone;
		BOOST_FOREACH(auto trans, newst->mTransitions){
			std::set<AFAStatePtr> cutset(trans.second.begin(),trans.second.end());
			transsetthisone.insert(std::make_pair(trans.first,cutset));
		}
		BOOST_FOREACH(auto t, currentst->mTransitions){
			std::string sym = t.first;
			std::set<AFAStatePtr> transset;
			BOOST_FOREACH(auto w, t.second){
			if(eqclass.find(w)==eqclass.end()){
				AFAStatePtr eqc = new AFAState(ORLit,w->mAMap);
				//tobedeleted.insert(eqc);
				eqc->mHMap=new z3::expr(*(w->mHMap));
				eqclass.insert(std::make_pair(w,eqc));
				transset.insert(eqc);
			}else{
				transset.insert(eqclass.find(w)->second);
				}
			}
			//insert to mappedone's transition only if this transset not already present on symbol sym from
			//this state..
			transsetthisone.insert(std::make_pair(sym,transset));
		}
		newst->mTransitions.clear();
		BOOST_FOREACH(auto t, transsetthisone){
			SetAFAStatesPtr tt(t.second.begin(),t.second.end());
			newst->mTransitions.insert(std::make_pair(t.first,tt));
		}
	}
	//Set init properly..
	if(init->mType==ORLit)
		newinit=eqclass.find(init)->second;
	else{//change the content of the transitions of OR/AND node..and keep newinit as the same init.
		//for each transition in this state construct
		//also remove this node from tobedeleted set..
		tobedeleted.erase(init);
				std::set<std::pair<std::string,std::set<AFAStatePtr>>,transitionscomparatorraw> transsetthisone;
				BOOST_FOREACH(auto t, init->mTransitions){
					std::set<AFAStatePtr> transset;
					BOOST_FOREACH(auto w, t.second){
						transset.insert(eqclass.find(w)->second);
					 }
					transsetthisone.insert(std::make_pair(t.first,transset));
				}
				init->mTransitions.clear();
				BOOST_FOREACH(auto w, transsetthisone){
					SetAFAStatesPtr tt(w.second.begin(),w.second.end());
					init->mTransitions.insert(std::make_pair(w.first,tt));
				}
				newinit=init;
	}
}



tm->mInit=newinit;
std::cout<<"Phase eqclass over"<<std::endl;
//tm->PrintToDot(std::string("Pass4PhaseEqClass.dot"));

//std::cin>>i;

/************NOw add self loops on those non-assume symbols whose freevar does not match with free var of current
 * state's mAMap..-- Should we also add assume symbols?? Lets keep that open for the time being..
 */


AFAStatePtr trueinit=NULL;
AFAStatePtr falseaccepted;
z3::expr truev=newinit->mAMap.ctx().bool_val(true);

	//Add self loops and
	//add self loops transitions (get added only on non-zero symbols):
 {
           std::set<std::string> allsyms;
           allsyms.insert(AFAut::mProgram->mAllSyms.begin(),AFAut::mProgram->mAllSyms.end());
		BOOST_FOREACH(auto w, newORLitSet)
			{
			SetAFAStatesPtr own;
			own.insert(w);
			if(w->mAMap.hash()==truev.hash()){//if valid
				std::cout<<"Found one"<<std::endl;
				trueinit=w;
			  BOOST_FOREACH(auto t, AFAut::mProgram->mAllSyms){
				 // if(w->mTransitions.find(t)==w->mTransitions.end()){//add only if that symbol is not present..
						w->mTransitions.insert(std::make_pair(t,own));
				//  }
			  }
			}else	if(w->HelperIsUnsat(w->mAMap)){
				falseaccepted=w;
				BOOST_FOREACH(auto t, AFAut::mProgram->mAllSyms)
				w->mTransitions.insert(std::make_pair(t,own));
			}else{
				std::set<z3::expr,mapexpcomparator> freevars = w->HelperGetFreeVars(w->mAMap);
				BOOST_FOREACH(auto t, AFAut::mProgram->mRWLHRHMap)
				{
                  // if(w->mTransitions.find(t.first)!=w->mTransitions.end())
                    //      continue;
					std::string sym = t.first;
					z3::expr lhs = std::get<0>(t.second);
					if(freevars.find(lhs)==freevars.end())//means not found as a free variable..
						{
						//add a self loop on this symbol to itself..
						w->mTransitions.insert(std::make_pair(sym,own));
						}
				}
				BOOST_FOREACH(auto t, AFAut::mProgram->mCASLHRHMap)
				{
                   //if(w->mTransitions.find(t.first)!=w->mTransitions.end())
                     // continue;
                   std::string sym = t.first;
                   z3::expr lhs = std::get<0>(t.second);
                   if(freevars.find(lhs)==freevars.end())//means not found as a free variable..
					{
						//add a self loop on this symbol to itself..
						w->mTransitions.insert(std::make_pair(sym,own));
					}
				}
		        BOOST_FOREACH(auto t, AFAut::mProgram->mAssumeLHRHMap)
				{
                  //if(w->mTransitions.find(t.first)!=w->mTransitions.end())
                   // continue;
		        	//add a self loop on this symbol to itself..
		        	w->mTransitions.insert(std::make_pair(t.first,own));
				}
			}

		}

 }

		//BUG: I was not adding a transition from newinit to trueinit..
		//If newinint is AND/OR then just add it as 0 labeled transition
		//if newinint is ORLit then create a new ANDOR state and add newinit and this one as transition on 0..v
 	 if(trueinit!=NULL)
 	 {
		if(newinit->mType!=ORLit){
			SetAFAStatesPtr trueinitset;
			trueinitset.insert(trueinit);
			newinit->mTransitions.insert(std::make_pair("0",trueinitset));
		}else{
			//create a new ANDOR node and add newinit and trueinit as its children..
			AFAStatePtr newnode = new AFAState(AND,newinit->mAMap);
			//tobedeleted.insert(newnode);
			newnode->mHMap=new z3::expr(*(newinit->mHMap));
			SetAFAStatesPtr newinset;
			newinset.insert(newinit);
			SetAFAStatesPtr trueinitset;
			trueinitset.insert(trueinit);
			newnode->mTransitions.insert(std::make_pair("0",newinset));
			newnode->mTransitions.insert(std::make_pair("0",trueinitset));
			newinit=newnode;
		}
 	 }
		tm->mInit=newinit;
		std::cout<<"Phase self loop addition over"<<std::endl;
		//tm->PrintToDot(std::string("Pass4PhaseSelfLoop.dot"));

		//Create generator


    AFAStatePtr falseerror=tm->Complement(falseaccepted);
	std::cout<<"Complement done "<<std::endl;
	//tm->PrintToDot(std::string("Pass4PhaseComplement.dot"));

return tm;

}

std::set<std::set<AFAStatePtr>> AFAut::GetConjunctedTransitions(std::set<AFAStatePtr> stateset, std::string sym){
			std::set<std::set<AFAStatePtr>> tobeaddedset;
			std::set<AFAStatePtr> emptyset;
			tobeaddedset.insert(emptyset);
			BOOST_FOREACH(auto state, stateset){
				if(state->mTransitions.find(sym)==state->mTransitions.end()){
					tobeaddedset.clear();//This is necessary to avoid executing the loop with partially filled tobeaddedset
					break;//means now we can not proceed to have an outgoing edge on this  symbol because at least one
					//does not have that..
				}
				std::set<std::set<AFAStatePtr>> tmptobeaddedset;
				BOOST_FOREACH(auto w, tobeaddedset){
					for(std::multimap<std::string, SetAFAStatesPtr>::iterator it= state->mTransitions.equal_range(sym).first; it!= state->mTransitions.equal_range(sym).second; it++)
					{
						std::set<AFAStatePtr> tmp(it->second.begin(),it->second.end());
						std::set<AFAStatePtr> tobeadded(w.begin(),w.end());
						tobeadded.insert(tmp.begin(),tmp.end());
						tmptobeaddedset.insert(tobeadded);
					}

				}
				tobeaddedset.clear();
				tobeaddedset.insert(tmptobeaddedset.begin(),tmptobeaddedset.end());

			}
			return tobeaddedset;
}


std::set<AFAStatePtr> AFAut::GetDisjunctedTransitions(std::set<AFAStatePtr> stateset, std::string sym){
			std::set<AFAStatePtr> tobeaddedset;
			BOOST_FOREACH(auto state, stateset){
				if(state->mTransitions.find(sym)!=state->mTransitions.end()){
					for(std::multimap<std::string,SetAFAStatesPtr>::iterator it= state->mTransitions.equal_range(sym).first;it!=state->mTransitions.equal_range(sym).second;it++)
					{
						SetAFAStatesPtr st = it->second;
						tobeaddedset.insert(st.begin(),st.end());
					}
				}
			}
			return tobeaddedset;
}


