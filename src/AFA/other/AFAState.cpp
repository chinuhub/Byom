/*
 * AFAState.cpp
 *
 *  Created on: 23-Aug-2015
 *      Author: jaganmohini
 */

#include "AFAState.h"
#include<boost/foreach.hpp>
#include<unordered_map>
#include<vector>
#include "AFAut.h"

bool mapstatecomparator::operator() (const AFAStatePtr& one, const AFAStatePtr& two) const
	{
		z3::expr onephi = (*one).mAMap;
		z3::expr twophi = (*two).mAMap;
		std::string& onestr = (*one).mRWord;
		std::string& twostr = (*two).mRWord;
		//bool res=z3::eq(onephi,twophi);
		if(onephi.hash()==twophi.hash())
			return onestr<twostr;
		else
			return (onephi.hash()<twophi.hash());

	}

bool transitionscomparator::operator() (const std::pair<std::string,SetAFAStatesPtr>& one, const std::pair<std::string,SetAFAStatesPtr>& two) const
	{
		std::string firststr = std::get<0>(one);
		std::string secondstr= std::get<0>(two);
		SetAFAStatesPtr firstset = std::get<1>(one);
		SetAFAStatesPtr secondset = std::get<1>(two);
		if(firststr.compare(secondstr)==0)
			return firstset<secondset;
		else
			return firststr<secondstr;

	}

bool transitionscomparatorraw::operator() (const std::pair<std::string,std::set<AFAStatePtr>>& one, const std::pair<std::string,std::set<AFAStatePtr>>& two) const
	{
		std::string firststr = std::get<0>(one);
		std::string secondstr= std::get<0>(two);
		std::set<AFAStatePtr> firstset = std::get<1>(one);
		std::set<AFAStatePtr> secondset = std::get<1>(two);
		if(firststr.compare(secondstr)==0)
			return firstset<secondset;
		else
			return firststr<secondstr;

	}


bool mapexpcomparator::operator() (const z3::expr& one, const z3::expr& two) const
  	{

  			return (one.hash()<two.hash());
  	}




bool AFAState::HelperIsUnsat(z3::expr formula)
	{
		z3::context& ctx = mAMap.ctx();
		z3::solver solv(ctx);
		z3::params pc(ctx);
		pc.set(":unsat-core",true);
		solv.set(pc);
		solv.add(formula);
		if(solv.check()==z3::check_result::unsat){
			return true;
		}else
			return false;

	}

bool AFAState::HelperIsValid(z3::expr formula)
	{
		z3::context& ctx = mAMap.ctx();
		z3::solver solv(ctx);
		z3::params pc(ctx);
		pc.set(":unsat-core",true);
		solv.set(pc);
		solv.add(!formula);
		if(solv.check()==z3::check_result::unsat){
			return true;
		}else
			return false;

	}

z3::expr AFAState::HelperSimplifyExpr(z3::expr exp)
	{
		z3::context& ctx = exp.ctx();
		z3::tactic t = z3::tactic(ctx,"simplify");
		z3::goal g(ctx);
		g.add(exp);
		z3::apply_result res = t(g);
		BOOST_ASSERT_MSG(res.size()>0,"Some problem with applying tactic");
		for(int i=0; i<res.size();i++){
				z3::goal r = res[i];
				BOOST_ASSERT_MSG(i==0,"This loop should run only once");
				//each goal r is of the form a and b and c (or literal a)..
				return r.as_expr();
		}

	}

std::set<z3::expr,mapexpcomparator> AFAState::HelperGetFreeVars(z3::expr& phi){
	if(phi.is_var()){
		BOOST_ASSERT_MSG(phi.is_var(),"Some problem: This must be var only");
		std::set<z3::expr,mapexpcomparator> res;
		if(phi.is_int()&&!phi.is_numeral()){//I dont know when will it come here.. this function needs more understanding of z3
			std::cout<<phi<<" is not numeral"<<std::endl;
			res.insert(phi);
		    return res;
		}else
			return res;
	}else if(phi.is_app()){
		std::set<z3::expr,mapexpcomparator> res;
		if(phi.is_int()&&!phi.is_numeral()){
			//std::cout<<phi<<" is var too "<<std::endl;
			res.insert(phi);
		}
		//std::cout<<phi<<" is app "<<std::endl;
		unsigned num = phi.num_args();
		for (unsigned i = 0; i < num; i++) {
			z3::expr arg = phi.arg(i);
	        std::set<z3::expr,mapexpcomparator> curr= HelperGetFreeVars(arg);
	        res.insert(curr.begin(),curr.end());
	     }
		return res;
	}else if(phi.is_quantifier()){
		//std::cout<<phi<<" is quantifier"<<std::endl;
		z3::expr arg = phi.body();
		return HelperGetFreeVars(arg);
	}else
		BOOST_ASSERT_MSG(false,"None type of expr found in freevar");


}

void AFAState::PassOne(std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& mAllStates){

#ifdef	DBGPRNT
	std::cout<<"Inside Pass one with phi ="<<mAMap<<" and type is "<<mType<<std::endl;
#endif
	if(mType==AND){
			//by this time we are sure that mPhi of this state is of the form a and b and c
			//extract a, b,c such clauses..
#ifdef	DBGPRNT
				std::cout<<" inside conjfillstate "<<this->mAMap<<std::endl;
#endif
				BOOST_ASSERT_MSG(mAMap.decl().decl_kind()==Z3_OP_AND," Expeceting a conjunction , failed.. SOme issue");
				z3::context& ctx= mAMap.ctx();
				z3::tactic t = z3::tactic(ctx,"tseitin-cnf");
				z3::goal g(ctx);
				g.add(mAMap);
				z3::apply_result res = t(g);
				SetAFAStatesPtr nextset;
				int cond=0;
				for(int i=0; i<res.size();i++){
				z3::goal r = res[i];
					BOOST_ASSERT_MSG(cond==0,"This loop should run only once");
					//each goal r is of the form a and b and c (or literal a)..
					z3::expr rexp = r.as_expr();
					int l = rexp.num_args();
					for(int p=0; p<l; p++){
						bool isPresent=false;
						z3::expr pass= rexp.arg(p);
						AFAStatePtr st = HelperAddStateIfAbsent(pass,mRWord,isPresent,mAllStates);//add to SeenSet if not.. else return the pointer..
						nextset.insert(st);
						if(!isPresent){
							mAllStates.insert(std::make_pair(st,st));
							st->PassOne(mAllStates);
						}
					}
					cond++;
				}
				//add HMap, by this time the returned state's must have proper HMap set as well..
				z3::expr trueexp = ctx.bool_val(true);
				BOOST_FOREACH(auto stp, nextset)
				{
					BOOST_ASSERT_MSG((*stp).mHMap!=NULL,"Some serious issue as by this time HMap of children must have been set");
					trueexp = trueexp && (*((*stp).mHMap));
				}
				trueexp=HelperSimplifyExpr(trueexp);
				mHMap = new z3::expr(trueexp);//delete it when removin the states.. i.e. in the destructor of this state..
				mTransitions.insert(std::make_pair("0",nextset));
	}else if(mType==OR){
		//by this time we are sure that mPhi of this state is of the form a or b or c
				(mAMap.decl().decl_kind()==Z3_OP_OR," Expeceting a disjunction, failed.. SOme issue");
				//extract a, b,c such clauses..
				z3::context& ctx= mAMap.ctx();
				z3::tactic t = z3::repeat(z3::tactic(ctx, "split-clause") | z3::tactic(ctx, "skip"));
				z3::goal g(ctx);
				g.add(mAMap);
				z3::apply_result res = t(g);
				//it must not throw exception because we know there exists at least one clause in mAMap by this point.
				SetAFAStatesPtr nextset;
				for(int i=0; i<res.size();i++){
					z3::goal r = res[i];
					//each goal r is of the form a , b or c.
					bool isPresent=false;
					z3::expr pass = r.as_expr();
					AFAStatePtr st = HelperAddStateIfAbsent(pass,mRWord,isPresent,mAllStates);//add to SeenSet if not.. else return the pointer..
					nextset.insert(st);
					if(!isPresent){
						mAllStates.insert(std::make_pair(st,st));
						st->PassOne(mAllStates);
					}
				}
				//add HMap, by this time the returned state's must have proper HMap set as well..
				z3::expr falseexp = ctx.bool_val(false);
				BOOST_FOREACH(auto stp, nextset)
				{
					BOOST_ASSERT_MSG((*stp).mHMap!=NULL,"Some serious issue as by this time HMap of children must have been set");
					falseexp = falseexp || (*((*stp).mHMap));
				}
				falseexp=HelperSimplifyExpr(falseexp);
				mHMap = new z3::expr(falseexp);//delete it when removin the states.. i.e. in the destructor of this state..
				mTransitions.insert(std::make_pair("0",nextset));

	}else if(mType==ORLit){
		//if mRWrod becomes empty then add it to acceptance state..(even before doing anything else in this loop
			if(mRWord.length()==0||mIsAccepted){
				//add this to accepting state and return with empty set..
				//this state must have been added to set of all states..
				mHMap = new z3::expr(mAMap);
				mIsAccepted=true;
				return;
			}
		//get free variables in mAMap;-1
		SetAFAStatesPtr nextset;
			std::set<z3::expr,mapexpcomparator> freevars=HelperGetFreeVars(mAMap);
#ifdef	DBGPRNT
			std::cout<<"Got following free vars"<<std::endl;
			BOOST_FOREACH(auto z, freevars)
			{
				std::cout<<"free var is "<<z<<std::endl;
			}
#endif
			z3::context& ctx= mAMap.ctx();
			//start with end of mRWord and keep on moving to front..
			int i=1;
			bool notasingleall=true;

		    for (std::string::reverse_iterator rit= mRWord.rbegin(); rit!=mRWord.rend(); ++rit,i++)
		    {
				bool notasingle=true;
		   	    std::string rest(mRWord.begin(),mRWord.end()-i);
				//for each character encountered..
		    	char c = *rit;
		    	std::string sym(1,c);
		    	if(AFAut::mProgram->mRWLHRHMap.find(sym)!=AFAut::mProgram->mRWLHRHMap.end()){
		    		//means it is a read/write symbol
		    		bool isPresent;
		    	//if does not conflict with the set -1 then add to mNonConflict set
		    		z3::expr left(std::get<0>((AFAut::mProgram->mRWLHRHMap.find(sym)->second)));
#ifdef	DBGPRNT
		    		std::cout<<"RW:lhs of exp is "<<left<<std::endl;
	#endif

		    		if(freevars.find(left)!=freevars.end()){
		    			//means this symbol conflict with phi and hence must be used for processing..
		    			notasingle=false;
		    			notasingleall=false;
		    			z3::expr right(std::get<1>((AFAut::mProgram->mRWLHRHMap.find(sym)->second)));
		    			z3::expr_vector src(ctx),dest(ctx);
		    			src.push_back(left);
		    			dest.push_back(right);
		    			z3::expr l1(mAMap.substitute(src,dest));
		    			l1=HelperSimplifyExpr(l1);
		    			bool isfalse=false;
		    			bool istrue=false;
		    			if(HelperIsUnsat(l1)){
		    				l1=ctx.bool_val(false);
		    				isfalse=true;
		    			}
		    			if(HelperIsValid(l1)){
		    				l1=ctx.bool_val(true);
		    				istrue=true;
		    			}
		    			AFAStatePtr p = HelperAddStateIfAbsent(l1,rest,isPresent,mAllStates);
		    			if(isfalse||(istrue&& rest.length()==0)){
		    				p->mIsAccepted=true;
		    				//On set of all symbols add self loop to p itself
		    			/*	BOOST_FOREACH(auto t, AFAut::mProgram->mAllSyms){
		    					HelperAddEdgeIfAbsent(p,p,t);
		    				}
*/
		    			}
		    			if(!isPresent){
		    				mAllStates.insert(std::make_pair(p,p));
		    				p->PassOne(mAllStates);
		    			}
		    			nextset.insert(p);
						//add HMap, by this time the returned state's must have proper HMap set as well..
						z3::context& ctx = mAMap.ctx();
						z3::expr trueexp = ctx.bool_val(true);
						BOOST_FOREACH(auto stp, nextset)
						{
							BOOST_ASSERT_MSG((*stp).mHMap!=NULL,"Some serious issue as by this time HMap of children must have been set");
							trueexp = trueexp && (*((*stp).mHMap));
						}
						trueexp=HelperSimplifyExpr(trueexp);
						mHMap = new z3::expr(trueexp);//delete it when removin the states.. i.e. in the destructor of this state..
		    			mTransitions.insert(std::make_pair(sym,nextset));
		    			break;
		    		}
		    	}else if(AFAut::mProgram->mCASLHRHMap.find(sym)!=AFAut::mProgram->mCASLHRHMap.end()){
		    		//means it is a cas symbol
		    		bool isPresent=false;
#ifdef	DBGPRNT
		    		std::cout<<"Found cas "<<sym<<std::endl;
#endif
		    		z3::expr left(std::get<0>(AFAut::mProgram->mCASLHRHMap.find(sym)->second));
#ifdef	DBGPRNT
		    		std::cout<<"CAS:lhs of exp is "<<left<<std::endl;
	#endif

		    		z3::expr l1(mAMap);
		    		z3::expr readarg(std::get<1>(AFAut::mProgram->mCASLHRHMap.find(sym)->second));
		    		//BEWARE that even if no substituteion takes place in cas we still need to conjunct assume part
		    		//with the input state's mAMap.
		    		notasingle=false;
		    		notasingleall=false;
		    		if(freevars.find(left)!=freevars.end()){
		    			//means this symbol conflict with phi and hence must be used for processing..
		    			z3::expr writearg(std::get<2>(AFAut::mProgram->mCASLHRHMap.find(sym)->second));
#ifdef	DBGPRNT
		    			std::cout<<"CAS:first arg of exp is "<<readarg<<std::endl;
		    			std::cout<<"CAS:second arg of exp is "<<writearg<<std::endl;
#endif
		    			z3::expr_vector src(ctx),dest(ctx);
		    			src.push_back(left);
		    			dest.push_back(writearg);
		    			z3::expr l3(mAMap.substitute(src,dest));
		    			l1=l3;
		    			l1=HelperSimplifyExpr(l1);
		    			}
		    			z3::expr l2(l1 && (left==readarg));
		    			l2=HelperSimplifyExpr(l2);
		    			bool isFalse=false;
		    			bool istrue = false;
		    			if(HelperIsUnsat(l2))
		    			{
		    				  l2=ctx.bool_val(false);
		    				  isFalse = true;
		    			}
		    			if(HelperIsValid(l2)){
		    				l2=ctx.bool_val(true);
		    				istrue=true;
		    			}
		    			AFAStatePtr p = HelperAddStateIfAbsent(l2,rest,isPresent,mAllStates);
		    			if(isFalse||(istrue&& rest.length()==0)){
		    				//On set of all symbols add self loop to p itself
#ifdef	DBGPRNT
		    				std::cout<<"inside cas, setting accepted state with amap as "<<mAMap<<std::endl;
#endif
		    				p->mIsAccepted=true;
		    				/*BOOST_FOREACH(auto t, AFAut::mProgram->mAllSyms){
		    					HelperAddEdgeIfAbsent(p,p,t);
		    				}*/

		    			}

		    			if(!isPresent){
		    				mAllStates.insert(std::make_pair(p,p));
		    			    p->PassOne(mAllStates);
		    			}
		    			nextset.insert(p);
						//add HMap, by this time the returned state's must have proper HMap set as well..
						z3::context& ctx = mAMap.ctx();
						z3::expr trueexp = ctx.bool_val(true);
						BOOST_FOREACH(auto stp, nextset)
						{
							BOOST_ASSERT_MSG((*stp).mHMap!=NULL,"Some serious issue as by this time HMap of children must have been set");
							trueexp = trueexp && (*((*stp).mHMap));
						}

						trueexp=HelperSimplifyExpr(trueexp);
						mHMap = new z3::expr(trueexp);//delete it when removin the states.. i.e. in the destructor of this state..

		    			mTransitions.insert(std::make_pair(sym,nextset));
		    			break;

		    	}else if(AFAut::mProgram->mAssumeLHRHMap.find(sym)!=AFAut::mProgram->mAssumeLHRHMap.end()){
		    		//means it is an assume symbol
		    		//get the second argument..
		    		notasingle=false;
		    		notasingleall=false;
		    		bool isPresent;
		    		z3::expr assumepsi(AFAut::mProgram->mAssumeLHRHMap.find(sym)->second);
		    		z3::expr combined(mAMap && assumepsi);
		    		combined=HelperSimplifyExpr(combined);
		    		bool isFalse=false;
		    		bool istrue =false;
		    		if(HelperIsUnsat(combined)){
		    			combined=ctx.bool_val(false);
		    			isFalse=true;
		    		}
		    		if(HelperIsValid(combined)){
		    			combined=ctx.bool_val(true);
		    			istrue=true;
		    		}
		    		AFAStatePtr p = HelperAddStateIfAbsent(combined,rest,isPresent,mAllStates);
		    		if(isFalse||(istrue&& rest.length()==0)){
		    			//On set of all symbols add self loop to p itself: Not now..
		    			p->mIsAccepted=true;
/*
	    				BOOST_FOREACH(auto t, AFAut::mProgram->mAllSyms){
	    					HelperAddEdgeIfAbsent(p,p,t);
	    				}
*/
		    		}

		    		if(!isPresent){
		    			mAllStates.insert(std::make_pair(p,p));
		    			p->PassOne(mAllStates);
		    		}
		    		nextset.insert(p);
					//add HMap, by this time the returned state's must have proper HMap set as well..
					z3::context& ctx = mAMap.ctx();
					z3::expr trueexp = ctx.bool_val(true);
					BOOST_FOREACH(auto stp, nextset)
					{
						BOOST_ASSERT_MSG((*stp).mHMap!=NULL,"Some serious issue as by this time HMap of children must have been set");
						trueexp = trueexp && (*((*stp).mHMap));
					}
					trueexp=HelperSimplifyExpr(trueexp);
					mHMap = new z3::expr(trueexp);//delete it when removin the states.. i.e. in the destructor of this state..

		    		mTransitions.insert(std::make_pair(sym,nextset));
		    		break;//This break is important
		    	}else
		    		BOOST_ASSERT_MSG(false,"Serious error, symbol  is neither r/w, lcas nor assume");
		    	//if control reaches here with notasingle=true it means this symbol was not confliciting with this state's amap
		    	//add self loop on this symbol
		    	//NB: dont do it here as anyway this is going to be added later in pass four.
		    			/*    if(notasingle)
		    			    {
		    					HelperAddEdgeIfAbsent(this,this,sym);
		    			    }*/
		    }//end for loop
		    if(notasingleall)//if this is the case then make it accepting state as well.
		    {
		       	mHMap = new z3::expr(mAMap);
		       	mIsAccepted=true;
		    }

	}else
		BOOST_ASSERT_MSG(false,"State should either be AND, OR or ORLit, some serious problem");
}



bool AFAState::HelperAddEdgeIfAbsent(AFAStatePtr src,AFAStatePtr dest,std::string sym){

	SetAFAStatesPtr srcset;
	srcset.insert(src);
	SetAFAStatesPtr dstset;
	dstset.insert(dest);

	for(std::multimap<std::string, SetAFAStatesPtr>::iterator it= src->mTransitions.equal_range(sym).first; it!=src->mTransitions.equal_range(sym).second; it++){
		SetAFAStatesPtr curr = it->second;
		if(curr==dstset)
			return false;
	}
	//if control reaches here means we could not find this edge.. hence add..
	src->mTransitions.insert(std::make_pair(sym,dstset));
	return true;
}


AFAStatePtr AFAState::HelperAddStateIfAbsent(z3::expr& phi,std::string& mRWord,bool& isPresent, std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& mAllStates){
//create appropriate state based on the type of phi..also check if it is already present or not..
	if(phi.decl().decl_kind()==Z3_OP_AND){
		AFAStatePtr st = new AFAState(AND,mRWord,phi);
		if(mAllStates.find(st)!=mAllStates.end()){
			delete st;
			st=mAllStates.find(st)->second;
			isPresent=true;
		}else{
			//mAllStates.insert(std::make_pair(st,st));-- we wont add it hee.. but after hmap is over for this state..
			isPresent=false;
		}
		return st;
	}else if(phi.decl().decl_kind()==Z3_OP_OR){
		AFAStatePtr st = new AFAState(OR,mRWord,phi);
		if(mAllStates.find(st)!=mAllStates.end()){
			delete st;
			st=mAllStates.find(st)->second;
			isPresent=true;
		}else{
			//mAllStates.insert(std::make_pair(st,st));-- same as above
			isPresent=false;
		}
		return st;
	}else{
		AFAStatePtr st = new AFAState(ORLit,mRWord,phi);
		if(mAllStates.find(st)!=mAllStates.end()){
			delete st;
			st=mAllStates.find(st)->second;
			isPresent=true;
		}else{
			//mAllStates.insert(std::make_pair(st,st));-- same as above
			isPresent=false;
		}
		return st;
	}

}


std::set<std::string> AFAState::getTransitionKeys(){
	std::set<std::string> keyset;
	BOOST_FOREACH(auto t, mTransitions){
		keyset.insert(t.first);
	}
	return keyset;
}
AFAState::~AFAState() {
	delete mHMap;
	mHMap=NULL;
}


