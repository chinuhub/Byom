/*
 * AFAGen.cpp
 *
 *  Created on: 15-Jun-2015
 *      Author: jaganmohini
 */

#include "AFAGen.h"
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>



class AFAState{

protected:

	AFAGen& mAFAGen;

public:
	//set of transitions going out of this state..
	std::multimap<std::string,SetAFAStatesPtr> mTransitions;

	bool mIsAccepted;
	//Assn associated with this state
	z3::expr mAMap;
	//residual string
	std::string mRWord;
	//Hoare assn associated with this state
	//z3::expr* mHMap;//NOte that we cant use rference here because then we wont be able to set it later,,
	//adn we cant use normal var becaus then it is like o argument constructor which odes not exist for z3::expr
	//and hence give error.. Hence w have to live with * declaration., or rathe rmake it shared_ptr;;
	z3::expr* mHMap;

	AFAState(std::string& wrd, z3::expr& phi, AFAGen& afag): mRWord(wrd), mAMap(phi), mAFAGen(afag){mIsAccepted=false;mHMap=NULL;}


	virtual SetAFAStatesPtr FillState(){}//NEED to fix it
	virtual SetAFAStatesPtr RelaxTransitions(){}
	virtual ~AFAState(){delete mHMap;mHMap=NULL;}

	z3::expr simplifyExpr(z3::expr exp)
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
	virtual void FillHMap(){

		if(mIsAccepted){
			BOOST_ASSERT_MSG(mHMap!=NULL," Some problem as HMap must have been set by now");
			return;
		}

		//BOOST_ASSERT_MSG(mTransitions.size()==1," Some problem as the size of transitions must be 1 for others");
		SetAFAStatesPtr nextset;
		BOOST_FOREACH(auto t, mTransitions)
		{
			nextset=t.second;
		}
		z3::context& ctx = mAMap.ctx();
		z3::expr trueexp = ctx.bool_val(true);
		BOOST_FOREACH(auto stp, nextset)
		{
			trueexp = trueexp && (*((*stp).mHMap));
		}
		trueexp=simplifyExpr(trueexp);
		mHMap = new z3::expr(trueexp);//delete it when removin the states.. i.e. in the destructor of this state..
	}

virtual SetAFAStatesPtr GetSucc(){
			if(mTransitions.size()==0){
				BOOST_ASSERT_MSG(mHMap!=NULL," Some problem as HMap must have been set by now");
				BOOST_ASSERT_MSG(mIsAccepted==true," Some problem as this state must be the accepting one");
				SetAFAStatesPtr nextset;
				return nextset;//emptyset
			}
			BOOST_ASSERT_MSG(mTransitions.size()==1," Some problem as the size of transitions must be 1 initially");
			SetAFAStatesPtr nextset;
			BOOST_FOREACH(auto t, mTransitions)
			{
				nextset=t.second;
			}
			return nextset;

		}

virtual vertex_t DotPrinter(Graph& g, std::map<AFAStatePtr,vertex_t,mapstatecomparator>& mapind){}

virtual void AddEdgeForEquals(){
	//dont add in graph if it is becoming tiring.. do it later when ina a good shape..
		SetAFAStatesPtr setnext;
		setnext.insert(this);

		BOOST_FOREACH(auto sp, mAFAGen.mAllStates)
		  {
			AFAStatePtr statep = sp.first;
			std::cout<<"Checking if "<< statep->mAMap<<" and "<<mAMap<<" are equal"<<std::endl;
				if(statep!=this){//ecluding itself
					if(eq(statep->mAMap,mAMap)){//&&eq(*(statep->mHMap),*(mHMap))){//where they are syntactically equal
						std::cout<<"Found "<< statep->mAMap<<" and "<<mAMap<<" are equal"<<std::endl;
						statep->mTransitions.insert(std::make_pair("0",setnext));
					}
			}

		 }



}

};

bool mapstatecomparator::operator() (const AFAStatePtr& one, const AFAStatePtr& two) const
	{
		z3::expr onephi = (*one).mAMap;
		z3::expr twophi = (*two).mAMap;
		std::string& onestr = (*one).mRWord;
		std::string& twostr = (*two).mRWord;
		bool res=z3::eq(onephi,twophi);
		if(onephi.hash()==twophi.hash())
			return onestr<twostr;
		else
			return (onephi.hash()<twophi.hash());

	}



bool mapexpcomparator::operator() (const z3::expr& one, const z3::expr& two) const
	{
			return (one.hash()<two.hash());
	}


class ConjANDState: public AFAState
{

public:
	ConjANDState(std::string& wrd, z3::expr& phi, AFAGen& afag): AFAState(wrd,phi,afag){}

	virtual vertex_t DotPrinter(Graph& g, std::map<AFAStatePtr,vertex_t,mapstatecomparator>& mapind){
					SetAFAStatesPtr nextset;
					//create a new vertes for this state..
					vertex_t vthis = boost::add_vertex(g);
					VertexProp vp;
					std::stringstream stream;
					stream << mAMap;
					vp.vertlabel=stream.str();
					stream.str(std::string());//erase the buffer
					stream<<((*mHMap));
					vp.xlabel=stream.str();
					vp.shape="rectangle";
					vp.color="yellow";
					g[vthis]=vp;

					BOOST_FOREACH(auto t, mTransitions)
					{
							nextset=t.second;
							std::string sym = t.first;
							EdgeProp edgep;
							edgep.label=sym;
							BOOST_FOREACH(auto st, nextset)
							{
								BOOST_ASSERT_MSG(mapind.find(st)!=mapind.end(),"Some problem as by now this must have been added");
								vertex_t& vert = mapind.find(st)->second;
								//creat a dot transitions from index to dotnum to the file..
								edge_t e;
								bool added;
								boost::tie(e,added)=boost::add_edge(vthis,vert,g);
								g[e]=edgep;
							}
					}
					return vthis;
		}

	virtual SetAFAStatesPtr RelaxTransitions(){
		//collect the assertion AHMap on every outgoing state of this stte..
		BOOST_ASSERT_MSG(mTransitions.size()==1," Some problem as the size of transitions must be 1 initially");
		SetAFAStatesPtr nextset;
		z3::context& ctx = mAMap.ctx();
		z3::solver solv(ctx);
		z3::params pc(ctx);
		pc.set(":unsat-core",true);
		solv.set(pc);

		BOOST_FOREACH(auto t, mTransitions)
		{
				nextset=t.second;
		}
		int i=0;
		std::map<z3::expr,AFAStatePtr,mapexpcomparator> labstatemapping;
		std::vector<z3::expr> assumptions;
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
			SetAFAStatesPtr newset;
			z3::expr_vector core = solv.unsat_core();
			for (unsigned i = 0; i < core.size(); i++) {

				z3::expr nm = core[i];
				BOOST_ASSERT_MSG(labstatemapping.find(nm)!=labstatemapping.end(),"Some problem this should not have happened");
				newset.insert(labstatemapping.find(nm)->second);
			    }
			mTransitions.clear();
			mTransitions.insert(std::make_pair("0",newset));//remove other elemennts fro mnext state..
			return newset;
		}else
			return nextset;

	}


	virtual SetAFAStatesPtr FillState(){
		//by this time we are sure that mPhi of this state is of the form a and b and c
		//extract a, b,c such clauses..
#if DEBUG_LEVEL==1
		std::cout<<" inside conjfillstate "<<this->mAMap<<std::endl;
#endif
		BOOST_ASSERT_MSG(mAMap.decl().decl_kind()==Z3_OP_AND," Expeceting a conjunction , failed.. SOme issue");
		z3::context& ctx= mAMap.ctx();
		z3::tactic t = z3::tactic(ctx,"tseitin-cnf");
		z3::goal g(ctx);
		g.add(mAMap);
		z3::apply_result res = t(g);
		SetAFAStatesPtr nextset;
		SetAFAStatesPtr returnset;
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
				AFAStatePtr st = mAFAGen.AddIfNotThere(pass,mRWord,isPresent);//add to SeenSet if not.. else return the pointer..
				nextset.insert(st);
				if(!isPresent){
					returnset.insert(st);
				}
			}
			cond++;
		}
		mTransitions.insert(std::make_pair("0",nextset));
		return nextset;//now we are returning nextset completely.. caller will check if elemen twas new or not..

		//create appropriate states(depending upon the nature of a,b,c with unchanged ResdWd and a,b,c as phi
		//check in mSeenMap if these states were already present..
		//if yes then get corresponding value from map..destroy newly made object and prepare a set of such states..
		//add to transition on nop symbol..to this set of states..
	}


};

class DisjANDState: public AFAState
{
public:


	DisjANDState(std::string& wrd, z3::expr& phi, AFAGen& afag): AFAState(wrd,phi,afag){}

	virtual vertex_t DotPrinter(Graph& g, std::map<AFAStatePtr,vertex_t,mapstatecomparator>& mapind){
					SetAFAStatesPtr nextset;
					//create a new vertes for this state..
					vertex_t vthis = boost::add_vertex(g);
					VertexProp vp;
					std::stringstream stream;
					stream << mAMap;
					vp.vertlabel=stream.str();
					stream.str(std::string());//erase the buffer
					stream<<((*mHMap));
					vp.xlabel=stream.str();
					vp.shape="rectangle";
					vp.color="cyan";
					g[vthis]=vp;
					BOOST_FOREACH(auto t, mTransitions)
					{
							nextset=t.second;
							std::string sym = t.first;
							EdgeProp edgep;
							edgep.label=sym;
							BOOST_FOREACH(auto st, nextset)
							{
								BOOST_ASSERT_MSG(mapind.find(st)!=mapind.end(),"Some problem as by now this must have been added");
								vertex_t& vert = mapind.find(st)->second;
								//creat a dot transitions from index to dotnum to the file..
								edge_t e;
								bool added;
								boost::tie(e,added)=boost::add_edge(vthis,vert,g);
								g[e]=edgep;
							}
					}
					return vthis;
		}

		virtual SetAFAStatesPtr RelaxTransitions(){
			BOOST_ASSERT_MSG(mTransitions.size()==1," Some problem as the size of transitions must be 1 initially");
			SetAFAStatesPtr nextset;
			//collect the assertion AHMap on every outgoing state of this stte..
			BOOST_FOREACH(auto t, mTransitions)
			{
					nextset=t.second;
			}
			return nextset;

			}

	virtual SetAFAStatesPtr FillState(){
			//by this time we are sure that mPhi of this state is of the form a or b or c
		BOOST_ASSERT_MSG(mAMap.decl().decl_kind()==Z3_OP_OR," Expeceting a disjunction, failed.. SOme issue");

			//extract a, b,c such clauses..
			z3::context& ctx= mAMap.ctx();
			z3::tactic t = z3::repeat(z3::tactic(ctx, "split-clause") | z3::tactic(ctx, "skip"));
			z3::goal g(ctx);
			g.add(mAMap);
			z3::apply_result res = t(g);
			//it must not throw exception because we know there exists at least one clause in mAMap by this point.
			SetAFAStatesPtr nextset;
			SetAFAStatesPtr returnset;
			for(int i=0; i<res.size();i++){
				z3::goal r = res[i];
				//each goal r is of the form a , b or c.
				bool isPresent=false;
				z3::expr pass = r.as_expr();
				AFAStatePtr st = mAFAGen.AddIfNotThere(pass,mRWord,isPresent);//add to SeenSet if not.. else return the pointer..
				nextset.insert(st);
				if(!isPresent){
					returnset.insert(st);
				}
			}
			mTransitions.insert(std::make_pair("0",nextset));
			return nextset;

			//create appropriate states(depending upon the nature of a,b,c with unchanged ResdWd and a,b,c as phi
			//check in mSeenMap if these states were already present..
			//if yes then get corresponding value from map..and prepare a set of such states..
			//add to transition on nop symbol..to this set of states..
		}

};
class LitORState: public  AFAState
{
public:

	LitORState(std::string& wrd, z3::expr& phi, AFAGen& afag): AFAState(wrd,phi,afag){}

	virtual vertex_t DotPrinter(Graph& g, std::map<AFAStatePtr,vertex_t,mapstatecomparator>& mapind){
					SetAFAStatesPtr nextset;
					//create a new vertes for this state..
					vertex_t vthis = boost::add_vertex(g);
					VertexProp vp;
					std::stringstream stream;
					stream << mAMap;
					vp.vertlabel=stream.str();
					stream.str(std::string());//erase the buffer
					stream<<((*mHMap));
					vp.xlabel=stream.str();
					if(mIsAccepted){
						vp.shape="doubleoctagon";
						vp.color="green";
					}else
					{
						vp.shape="octagon";
						vp.color="green";
					}
					g[vthis]=vp;
					BOOST_FOREACH(auto t, mTransitions)
					{
							nextset=t.second;
							std::string sym = t.first;
							EdgeProp edgep;
							edgep.label=sym;
							BOOST_FOREACH(auto st, nextset)
							{
								BOOST_ASSERT_MSG(mapind.find(st)!=mapind.end(),"Some problem as by now this must have been added");
								vertex_t& vert = mapind.find(st)->second;
								//creat a dot transitions from index to dotnum to the file..
								edge_t e;
								bool added;
								boost::tie(e,added)=boost::add_edge(vthis,vert,g);
								g[e]=edgep;
							}
					}
					return vthis;
		}

	virtual SetAFAStatesPtr RelaxTransitions(){

		if(mTransitions.size()==0){
						BOOST_ASSERT_MSG(mHMap!=NULL," Some problem as HMap must have been set by now");
						BOOST_ASSERT_MSG(mIsAccepted==true," Some problem as this state must be the accepting one");
						SetAFAStatesPtr nextset;
						return nextset;//emptyset
					}

		BOOST_ASSERT_MSG(mTransitions.size()==1," Some problem as the size of transitions must be 1 initially");
		SetAFAStatesPtr nextset;
		if(mIsAccepted)
			return nextset;//if accepted state then no outgoing transiiton hence return empty..
		//collect the assertion AHMap on every outgoing state of this stte..
		BOOST_FOREACH(auto t, mTransitions)
		{
				nextset=t.second;
		}

		z3::context& ctx = mAMap.ctx();
		z3::solver solv(ctx);
		z3::params pc(ctx);
		pc.set(":unsat-core",true);
		solv.set(pc);
		int i=0;
		z3::expr formula = (mAMap!=(*mHMap));
		solv.add(formula);
		if(solv.check()==z3::check_result::unsat){
		//means amap and hmap are same and hence no need to follow the link..
			mTransitions.clear();
			//also make it an accepted state
			mIsAccepted = true;
			SetAFAStatesPtr tmp;
			return tmp;//empty set.
		}else
			return nextset;
	}


	virtual SetAFAStatesPtr FillState(){
		//if mRWrod becomes empty then add it to acceptance state..(even before doing anything else in this loop
		SetAFAStatesPtr returnset;
		if(mRWord.length()==0||mIsAccepted){
			//add this to accepting state and return with empty set..
			//this state must have been added to set of all states..
			mHMap = new z3::expr(mAMap);
			mIsAccepted=true;
			return returnset;//emptyset
		}
	//get free variables in mAMap;-1
		SetAFAStatesPtr nextset;
#if DEBUG_LEVEL==1
		std::cout<<"About to get the free var of "<<mAMap<<std::endl;
#endif
		std::set<z3::expr,mapexpcomparator> freevars=mAFAGen.GetFreeVars(mAMap);
#if DEBUG_LEVEL==1
		std::cout<<"Got following free vars"<<std::endl;
		BOOST_FOREACH(auto z, freevars)
		{
			std::cout<<"free var is "<<z<<std::endl;
		}
#endif
		z3::context& ctx= mAMap.ctx();
		//start with end of mRWord and keep on moving to front..
		int i=1;
	    for (std::string::reverse_iterator rit= mRWord.rbegin(); rit!=mRWord.rend(); ++rit,i++)
	    {
	   	    std::string rest(mRWord.begin(),mRWord.end()-i);
			//for each character encountered..
	    	char c = *rit;
	    	std::string sym(1,c);
	    	if(mAFAGen.mProgram.mRWLHRHMap.find(sym)!=mAFAGen.mProgram.mRWLHRHMap.end()){
	    		//means it is a read/write symbol
	    		bool isPresent;
	    	//if does not conflict with the set -1 then add to mNonConflict set
	    		z3::expr left(std::get<0>((mAFAGen.mProgram.mRWLHRHMap.find(sym)->second)));
#if DEBUG_LEVEL==1
	    		std::cout<<"RW:lhs of exp is "<<left<<std::endl;
#endif
	    		if(freevars.find(left)!=freevars.end()){
	    			//means this symbol conflict with phi and hence must be used for processing..
	    			z3::expr right(std::get<1>((mAFAGen.mProgram.mRWLHRHMap.find(sym)->second)));
	    			z3::expr_vector src(ctx),dest(ctx);
	    			src.push_back(left);
	    			dest.push_back(right);
	    			z3::expr l1(mAMap.substitute(src,dest));
	    			l1=simplifyExpr(l1);
	    			bool isfalse=false;
	    			if(IsUnsat(l1)){
	    				l1=ctx.bool_val(false);
	    				isfalse=true;
	    			}
	    			AFAStatePtr p = mAFAGen.AddIfNotThere(l1,rest,isPresent);
	    			if(isfalse)
	    				p->mIsAccepted=true;
	    			if(!isPresent){
	    				returnset.insert(p);
	    			}
	    			nextset.insert(p);
	    			mTransitions.insert(std::make_pair(sym,nextset));
	    			return nextset;
	    		}
	    	}else if(mAFAGen.mProgram.mCASLHRHMap.find(sym)!=mAFAGen.mProgram.mCASLHRHMap.end()){
	    		//meand it is a cas symbol
	    		std::cout<<"Found cas "<<sym<<std::endl;
	    		z3::expr left(std::get<0>(mAFAGen.mProgram.mCASLHRHMap.find(sym)->second));
#if DEBUG_LEVEL==1
	    		std::cout<<"CAS:lhs of exp is "<<left<<std::endl;
#endif
	    		std::cout<<"CAS:lhs of exp is "<<left<<std::endl;
	    		if(freevars.find(left)!=freevars.end()){
	    			bool isPresent;
	    			//means this symbol conflict with phi and hence must be used for processing..
	    			z3::expr writearg(std::get<2>(mAFAGen.mProgram.mCASLHRHMap.find(sym)->second));
	    			z3::expr readarg(std::get<1>(mAFAGen.mProgram.mCASLHRHMap.find(sym)->second));
	    			std::cout<<"CAS:first arg of exp is "<<readarg<<std::endl;
	    			std::cout<<"CAS:second arg of exp is "<<writearg<<std::endl;
	    			z3::expr_vector src(ctx),dest(ctx);
	    			src.push_back(left);
	    			dest.push_back(writearg);
	    			z3::expr l1(mAMap.substitute(src,dest));
	    			l1=simplifyExpr(l1);
	    			z3::expr l2(l1 && (left==readarg));
	    			l2=simplifyExpr(l2);
	    			bool isFalse=false;
	    			if(IsUnsat(l2))
	    			{
	    				  l1=ctx.bool_val(false);
	    				  isFalse = true;
	    			}
	    			AFAStatePtr p = mAFAGen.AddIfNotThere(l2,rest,isPresent);
	    			if(isFalse)
	    				p->mIsAccepted=true;
	    			if(!isPresent){
	    				returnset.insert(p);
	    			}
	    			nextset.insert(p);
	    			mTransitions.insert(std::make_pair(sym,nextset));
	    			return nextset;
	    		}
	    	}else if(mAFAGen.mProgram.mAssumeLHRHMap.find(sym)!=mAFAGen.mProgram.mAssumeLHRHMap.end()){
	    		//means it is an assume symbol
	    		//get the second argument..
	    		bool isPresent;
	    		z3::expr assumepsi(mAFAGen.mProgram.mAssumeLHRHMap.find(sym)->second);
	    		z3::expr combined(mAMap && assumepsi);
	    		combined=simplifyExpr(combined);
	    		bool isFalse=false;
	    		if(IsUnsat(combined)){
	    			combined=ctx.bool_val(false);
	    			isFalse=true;
	    		}
	    		AFAStatePtr p = mAFAGen.AddIfNotThere(combined,rest,isPresent);
	    		if(isFalse)
	    			p->mIsAccepted=true;
	    		if(!isPresent){
	    			returnset.insert(p);
	    		}
	    		nextset.insert(p);
	    		mTransitions.insert(std::make_pair(sym,nextset));


	    		return nextset;
	    	}

	    }
	    //if control reaches here means for every symbol in mRegWd of this state assertio is stable with all..
	    //make this a accepting state.-- also update HMap

	    mHMap = new z3::expr(mAMap);
	    mIsAccepted=true;
	    return returnset;//emptyset
	}

	bool IsUnsat(z3::expr formula)
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

	void AddEdges(std::set<LitORState*, mapstatecomparator> setlitors, Graph& g,std::map<AFAStatePtr, vertex_t,mapstatecomparator>& indmap)
	{
		//Pass 1: if HMap(this) is unsat and AMap(this) is not unsat then
		//for every other state s' in setlitors(excluding itself)
		//check the validity of amap(s')\limp amap(this).. by checking unsat of !amap(s')\limp amap(this)
		//if unsat (means valid) then add an edge from s' to s with nop symbol..
		SetAFAStatesPtr setnext;
		setnext.insert(this);
		vertex_t dest = indmap.find(this)->second;

				EdgeProp edgep;
				edge_t e;
				bool added;

		if(IsUnsat(*mHMap))
		{
			if(!IsUnsat(mAMap))
			{
				bool result = IsUnsat(!mAMap);
				if(!result)//means mAMap is not valid.. then every one will imply true.. in this case add epsilon from init to this state only..to avoid explosion..
				{

					/*BOOST_FOREACH(auto statep, setlitors)
					  {
						if(statep!=this){//ecluding itself
							if(!IsUnsat(statep->mAMap)){//excluding where redundant edges will be added
								z3::expr formula = implies((statep->mAMap), mAMap);//SURPRISE: why z3::implies give
								//overloading function unknown error.. FTB I have removed it..
								z3::expr negf(!formula);
								if(IsUnsat(negf)){//means a=>b is valid
									statep->mTransitions.insert(std::make_pair("0",setnext));
									//add and edge in dot graph as well..
									vertex_t src = indmap.find(statep)->second;
									edgep.label="0";
									boost::tie(e,added)=boost::add_edge(src,dest,g);
									g[e]=edgep;
								}
							}
						}
					}*/
					BOOST_FOREACH(auto statep, setlitors)
										  {
											if(statep!=this){//ecluding itself
												if(eq(statep->mAMap,mAMap)){
							 					statep->mTransitions.insert(std::make_pair("0",setnext));
														//add and edge in dot graph as well..
														vertex_t src = indmap.find(statep)->second;
														edgep.label="0";
														boost::tie(e,added)=boost::add_edge(src,dest,g);
														g[e]=edgep;
													}
												}
											}

				}else{
					//add epsilon edge from init state to this state only..because this amap is valid..
					mAFAGen.init->mTransitions.insert(std::make_pair("0",setnext));
					vertex_t src = indmap.find(mAFAGen.init)->second;
					edgep.label="0";
					boost::tie(e,added)=boost::add_edge(src,dest,g);
					g[e]=edgep;
				}
			}
		}

		//Pass 2:get FV of AMap(this).. for every symbol in mLRWMap and mLCASMap check if their second's first element
		//(i.e.) write variable is in fvset of this amap.. if not..then add self loop on that symbol(first) to the same
		//state..[This FV might be stored to sppedup if that matters at all]

		std::set<z3::expr,mapexpcomparator> freevars = mAFAGen.GetFreeVars(mAMap);
		BOOST_FOREACH(auto t, mAFAGen.mProgram.mRWLHRHMap)
		{
			std::string sym = t.first;

			z3::expr lhs = std::get<0>(t.second);
			if(freevars.find(lhs)==freevars.end())//means not found as a free variable..
			{
				//add a self loop on this symbol to itself..
				mTransitions.insert(std::make_pair(sym,setnext));
				//add an edge in dot graph as welll..
				edgep.label=sym;
				boost::tie(e,added)=boost::add_edge(dest,dest,g);
				g[e]=edgep;
			}
		}

		BOOST_FOREACH(auto t, mAFAGen.mProgram.mCASLHRHMap)
		{
			std::string sym = t.first;

			z3::expr lhs = std::get<0>(t.second);
			if(freevars.find(lhs)==freevars.end())//means not found as a free variable..
			{
				//add a self loop on this symbol to itself..
				mTransitions.insert(std::make_pair(sym,setnext));
				//add an edge in dot graph as welll..
				edgep.label=sym;
				boost::tie(e,added)=boost::add_edge(dest,dest,g);
				g[e]=edgep;
			}
		}
//MOVED after construction of NFA from AFA to minimize number of unnecessary self loops and transitions..
		//PASS 3: if HMap(s) is unsat then for every symbol in mAssume also add a self loop..because A^B=>B hence
		//having assume here will still derive false as wp..
		BOOST_FOREACH(auto t, mAFAGen.mProgram.mAssumeLHRHMap)
				{
					std::string sym = t.first;
						//add a self loop on this symbol to itself..
						mTransitions.insert(std::make_pair(sym,setnext));
						//add an edge in dot graph as welll..
						edgep.label=sym;
						boost::tie(e,added)=boost::add_edge(dest,dest,g);
						g[e]=edgep;


				}

		//if(IsUnsat(!mAMap))// This holds even if mAMap is not true..because P^Q always implies Q no matter what is P
		{
			//Next:: if amap is true then search for all states in mAallStates to check if amap of that
			//matches with t.second if yes then add an edge from this state to that state on sym..
			BOOST_FOREACH(auto t, mAFAGen.mProgram.mAssumeLHRHMap)
							{
								std::string sym = t.first;
								z3::expr right = t.second;
								BOOST_FOREACH(auto v, mAFAGen.mAllStates){
									if(v.first==this)//except this state
										continue;
									AFAStatePtr stt = v.first;
									if(eq(stt->mAMap, right)){
										  SetAFAStatesPtr ptrset;
										  ptrset.insert(stt);
										  mTransitions.insert(std::make_pair(sym,ptrset));
									 }
								}

							}

		}
		//PASS4: It might add more edges but will surely be expensive because of lot of unsat checking and wp construction..
		//hence skipping for the time being..IN this we will add and edge labeled 'a' from s1 to s2 if wp(s1.ampa,a)=>s2.amap
		//and HMap(s2) is unsat..or
		//wp(s1.amap,a)=s2.amap
		//ONE: at least add edges between s and s' such that s.amap=s'.amap and hmap' are also same..
		//done in the base class function..


	}//close function addedge
};


AFAGen::AFAGen(std::string& w, z3::expr& phi, Program& p): mWord(w),mPhi(phi),mProgram(p){

}


template <class Name>
class myEdgeWriter {
  public:
         myEdgeWriter(Name _name) : name(_name) {}
              template <class VertexOrEdge>
                     void operator()(std::ostream& out, const VertexOrEdge& v) const {
                                   out << "[label=\"" << name[v].label << "\"]";
                                        }
  private:
                   Name name;
};


template <class Name>
class myVertWriter {
  public:
         myVertWriter(Name _name) : name(_name) {}
              template <class VertexOrEdge>
                     void operator()(std::ostream& out, const VertexOrEdge& v) const {
                                   out << "[label=\"" << name[v].vertlabel << "\", color="<<name[v].color<<",shape="<<name[v].shape<<"]";
                                        }
  private:
                   Name name;
};

//Main function responsible for invoking passes..
struct fa* AFAGen::ProcessProof(int count)
{
	    //for mPhi, create a state appropriately.., add this state to a worklist.. and repeat until worklist is empty..
		//inside loop, remove an element from this worklist and invoke the function FillState on it..
		//whatever is returned is added to the worklist.
		z3::context& ctx= mPhi.ctx();

		z3::tactic t = z3::tactic(ctx,"tseitin-cnf");
		z3::goal g(ctx);
		g.add(mPhi);
		z3::apply_result res = t(g);
		if(res.size()>1){
			//then it is surely of or type.. claus 1 or clause 2.. create an DisjAND state
			z3::expr formula = g.as_expr();
			AFAStatePtr st = new DisjANDState(mWord,formula,*this);
			PassOne(st);
			init = st;
		}else if(res.size()==1){
			z3::expr formula = res[0].as_expr();
			if(formula.decl().decl_kind()==Z3_OP_AND)
			{
				AFAStatePtr st = new ConjANDState(mWord,formula,*this);
				PassOne(st);
				init = st;
			}else if(formula.decl().decl_kind()==Z3_OP_OR)
			{
				AFAStatePtr st = new DisjANDState(mWord,formula,*this);
				PassOne(st);
				init = st;
			}else
			{
				AFAStatePtr st = new LitORState(mWord,formula,*this);
				PassOne(st);
				init = st;
			}

		}
		std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator> seenStates;
		PassTwo(init,seenStates);
		//every state that is in mAllState but not in seenstate must have been removed by exploration (pruning)
		//call delete on all those states.. so that we have correct set of states
		std::set<AFAStatePtr,mapstatecomparator> unreachable;
		BOOST_FOREACH(auto t, mAllStates){
			AFAStatePtr el = t.first;
			if(seenStates.find(el)==seenStates.end()){//because of find I cant remove the element in place..
				//means this element was not reachable after pruning of edges..
				unreachable.insert(el);
			}
		}
		BOOST_FOREACH(auto t, unreachable)
		{
			delete t;
		}
		mAllStates.clear();//remove all pointers..
		//copy pointers from seenStates to this set to ensure consistency.. as this will be removed in the destructor.
		mAllStates.insert(seenStates.begin(),seenStates.end());


 	    Graph graph;
 	    std::map<AFAStatePtr,vertex_t,mapstatecomparator> mapindex;
 	    PassThree(init,graph,mapindex);//fill the graph object


  	    myEdgeWriter<Graph> ew(graph);
 	    myVertWriter<Graph> vw(graph);
 	    //IMPO we need to put writers after graph is filled.

#ifdef DEBUG_FILE
 	    std::string fname= "DOTOutput"+std::to_string(count)+".dot";
 	    std::ofstream outf(fname);
 	    write_graphviz(outf, graph, vw,ew);
#endif
 	    //ANother pass to have more edges
   	    PassFour(graph,mapindex);

   	    //not exactly a pass but linking syntactically equivalanet amap nodes..
/*
   	    BOOST_FOREACH(auto t, mAllStates){
   	 			AFAStatePtr el = t.first;
   	 			el->AddEdgeForEquals();
   	 		}
*/

 	    std::cout<<"About to convert to NFA"<<std::endl;
		struct fa* nfa = ConvertToNFA(init);;
		return nfa;
}


/**
 * Construction pass of the AFA, recursive function : interestingly it constructs forward and then in a backward
 * manner pulls the HMap assigned to each state to get back to the initial state.. this seems a nice approach of
 * combining both phases..
 */
void AFAGen::PassOne(AFAStatePtr& state)
{

	//call fillState on state.. get all next states which are not yet explored..
	//those explored must have their HMap set as well..
	#if DEBUG_LEVEL==1
	std::cout<<"phi is PASSONE: "<<state->mAMap<<" with word as "<<state->mRWord<<std::endl;
	#endif
	SetAFAStatesPtr setelem = state->FillState();
	BOOST_FOREACH(auto st, setelem)
	{
		if(mAllStates.find(st)==mAllStates.end())//means this state has not been explored yet
		{
			PassOne(st);
			//st->FillHMap();//to fill the HMap of this state.. its completion guarantees that all its children have
			//completed too..
		}
	}
	state->FillHMap();//complete current state's HMAP too..
	BOOST_ASSERT_MSG(mAllStates.find(state)==mAllStates.end(),"Some error");//this must not have been added till now..
	mAllStates.insert(std::make_pair(state,state));

}

/**
 * Pass to collapse collapsible edges..
 */
void AFAGen::PassTwo(AFAStatePtr& state, std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& seenStates)
{
	SetAFAStatesPtr setelem = state->RelaxTransitions();
		BOOST_FOREACH(auto st, setelem)
		{
			if(seenStates.find(st)==seenStates.end())//means this state has not been explored yet
			{
				PassTwo(st, seenStates);
			}
		}
		BOOST_ASSERT_MSG(seenStates.find(state)==seenStates.end(),"Some error");//this must not have been added till now
		seenStates.insert(std::make_pair(state,state));

}

/**
 * Pass to create dot file of AFA
 */
void AFAGen::PassThree(AFAStatePtr& state, Graph& g, std::map<AFAStatePtr, vertex_t,mapstatecomparator>& indmap)
{
	SetAFAStatesPtr setelem = state->GetSucc();
			BOOST_FOREACH(auto st, setelem)
			{
				if(indmap.find(st)==indmap.end())//means this state has not been explored yet
				{
					PassThree(st, g,indmap);
				}
			}
			BOOST_ASSERT_MSG(indmap.find(state)==indmap.end(),"Some error");//this must not have been added till now
			//now all successors are ready (have been added to graph) so safe to call PrintDot on this node..
			vertex_t ver = state->DotPrinter(g, indmap);
			indmap.insert(std::make_pair(state,ver));

}


/**
 * Pass to add edges and loops in AFA.. note that these are added only from and to litor type of states.
 */
void AFAGen::PassFour (Graph& g,std::map<AFAStatePtr,vertex_t,mapstatecomparator>& mapindex)
{
	std::set<LitORState*, mapstatecomparator> setderived;// fine to use the same comparator because in a function of
	//base class pointer we can safely pass derived class objects.
	//extract all those states from AFA which are of type LitOrState
	BOOST_FOREACH(auto stuple, mAllStates){
		AFAStatePtr st= stuple.first;
		LitORState* litors = dynamic_cast<LitORState*>(st);
		if(litors!=NULL)
			setderived.insert(litors);

	}
	//For each of these states call function (add loopedges) defined in LitOrState class, along with the set as well..
	BOOST_FOREACH(auto state, setderived)
	{
		state->AddEdges(setderived,g,mapindex);//it is fine because this function is not going to remove any state form setderivedset
		//hence no fear of getting invalidated pointers..
	}

}





/*************************************Helper function******************************************************/
typedef std::pair<struct autstate*, SetAFAStatesPtr> AutAFACmb;
class myTransition{
public:
	struct autstate* to;
	char csym;
};

struct transcomp{
	bool operator() (const myTransition& one, const myTransition& two) const
		{
			if(one.to==two.to)
				return one.csym<two.csym;
			else
				return one.to<two.to;

		}
};

bool checksmall(SetAFAStatesPtr& one, SetAFAStatesPtr& two){
		//are of same length.., as both are ordered.. if same pointers were added to them they must be in same order..
		//use this to check of < ..


		SetAFAStatesPtr::iterator twoitb = two.begin();
		for(SetAFAStatesPtr::iterator oneitb = one.begin(); oneitb!=one.end();oneitb++,twoitb++)
		{
			if(*oneitb == *twoitb)
				continue;
			else
				return ((*oneitb)<(*twoitb));
		}
		return false;
	}

struct newcombcomparator{

	bool operator() (const AutAFACmb* one, const AutAFACmb*  two) const
	{
		SetAFAStatesPtr oneset = one->second;
		SetAFAStatesPtr twoset = two->second;
		if(oneset.size()!=twoset.size())
			return oneset.size()<twoset.size();
		else
			return checksmall(oneset,twoset);

	}
};


std::vector<SetAFAStatesPtr> AFAGen::getValues(std::multimap<std::string, SetAFAStatesPtr>& map, std::string& key)
{
    std::vector<SetAFAStatesPtr> result;
    typedef std::multimap<std::string, SetAFAStatesPtr>::iterator imm;
    std::pair<imm, imm> range =map.equal_range(key);
    for (imm i = range.first; i != range.second; ++i)
        result.push_back(i->second);
    return result;
}




struct fa* AFAGen::ConvertToNFA(AFAStatePtr inits)
{

	std::map<AutAFACmb*, AutAFACmb*,newcombcomparator> SeenMap;
	std::vector<std::pair<struct autstate*, struct autstate*> > epsilonedges;
	std::map<struct autstate*, std::set<myTransition,transcomp>> transmap;
	SetAFAStatesPtr ini;
	ini.insert(inits);
	struct fa* merged = fa_make_empty();
	//create a new state, init
	struct autstate* init = add_autstate(merged,0);//add a non-accepting state to this automaton
	//set init as initial state of merged
	merged->initial=init;
	//map init to initset
	AutAFACmb* initinfo = new AutAFACmb;
	initinfo->first = init;
	initinfo->second = ini;
	SeenMap[initinfo]=initinfo;
	//put init to a worklist, arraylist of AutAFACmb*
	std::vector<AutAFACmb*> worklist;
	worklist.push_back(initinfo);

	while(worklist.size()!=0)
		{

				AutAFACmb* picked = worklist.back();
				worklist.pop_back();
				//get state of actual nfa.
				struct autstate* nfastate = (picked)->first;
				SetAFAStatesPtr productset = picked->second;
				//check if nfastae should be set as an accepted state or not..by checking if
				//every AFAState in productstate is an accepting state or not.

				bool isAccepting= true;
				BOOST_FOREACH(auto st, productset)
				{
					isAccepting = isAccepting&&(st->mIsAccepted);
				}
				if(isAccepting)
				{
					nfastate->accept=1;
				}
				BOOST_FOREACH(auto sym, mProgram.mAllSyms)
				{

					char csym = sym.at(0);
					BOOST_FOREACH(AFAStatePtr state, productset)
						{
							if(state->mTransitions.find(sym)==state->mTransitions.end())
								continue; //means no outgoing transition from this state oon sym hence continue;
						}
					std::vector<SetAFAStatesPtr> setOfSets;

					BOOST_FOREACH(AFAStatePtr state, productset)
						{
						//if we reached here it implies taht every state in productset has atleast one outgoing transition
						//on sym.. now we need to collect them and make their products..
						//suppose s1 on a goes to two different set of states.. then it will return two set of SetAFAStatePtr
						//which then must be combined with others to create product sets..
						//each one of them then will be assigned a autstate* if not already present and returned to this
						//funciton.. which will then add transitions to them.. and add newly added autstate* along with their
						//product set to worklist..
						std::vector<SetAFAStatesPtr> thisstate = getValues(state->mTransitions,sym);
					    if(setOfSets.size()==0)
					    	setOfSets.insert(setOfSets.begin(),thisstate.begin(),thisstate.end());
					    else{
					    	std::vector<SetAFAStatesPtr> result;
					    	BOOST_FOREACH(SetAFAStatesPtr setp, setOfSets)
								{

					    			BOOST_FOREACH(SetAFAStatesPtr setp2, thisstate)
										{
					    				SetAFAStatesPtr tmp(setp);
					    				tmp.insert(setp2.begin(),setp2.end());
					    				result.push_back(tmp);
										}
								}
					    	setOfSets=result;
					     }
						}//done with traversing each state in productset..

						BOOST_FOREACH(SetAFAStatesPtr setstates, setOfSets){
							AutAFACmb* newinfo = new AutAFACmb;
 							newinfo->second = setstates;
							if(SeenMap.find(newinfo)!=SeenMap.end())
							{
								//means we found some mapping for this..
								AutAFACmb* tmpp = SeenMap.find(newinfo)->second;

								delete newinfo;
								newinfo = tmpp;
							}else
							{
								//create a new automata state..
								struct autstate* news= add_autstate(merged,0);//add a non-accepting state to this automaton
								newinfo->first = news;
								SeenMap.insert(std::make_pair(newinfo,newinfo));
								//add to worklist as well..
								worklist.push_back(newinfo);
							}
								myTransition tr;
								tr.to=newinfo->first;
								tr.csym=csym;
								std::set<myTransition,transcomp> stset;
								if(transmap.find(nfastate)==transmap.end()){

								}else{
									stset=transmap.find(nfastate)->second;
									transmap.erase(nfastate);
								}
								if(csym!='0')
									stset.insert(tr);
								else
									epsilonedges.push_back(std::make_pair(nfastate,newinfo->first));

								transmap.insert(std::make_pair(nfastate,stset));

						}
				}

		}//end of while loop
std::cout<<"Total number of states in aut are "<<transmap.size()<<std::endl;
std::cout<<"Total epsilon pairs are "<<epsilonedges.size()<<std::endl;
	//now use epsilon edges and transmap to generate epsilon closure
	bool ischanged = true;
	while(ischanged){
		ischanged=false;
		BOOST_FOREACH(auto t, epsilonedges)
			{
				//get transition set for t.first
				BOOST_ASSERT_MSG(transmap.find(t.first)!=transmap.end(),"SOme serious error check here");
				std::set<myTransition,transcomp> setone = transmap.find(t.first)->second;
				BOOST_ASSERT_MSG(transmap.find(t.second)!=transmap.end(),"SOme serious error check here");
				std::set<myTransition,transcomp> settwo = transmap.find(t.second)->second;
				int sizeone=setone.size();
				/*std::cout<<"size of one  is "<<sizeone<<std::endl;
				BOOST_FOREACH(auto p, setone)
				{
					std::cout<<"To "<<p.to<<" with symbol as "<<p.csym<<std::endl;
				}
				std::cout<<"size of two is "<<settwo.size()<<std::endl;
				BOOST_FOREACH(auto p, settwo)
				{
									std::cout<<"To "<<p.to<<" with symbol as "<<p.csym<<std::endl;
									setone.insert(p);
				}*/
				setone.insert(settwo.begin(),settwo.end());
				/*std::cout<<"size of one  after addition is "<<setone.size()<<std::endl;
				BOOST_FOREACH(auto p, setone)
								{
									std::cout<<"To "<<p.to<<" with symbol as "<<p.csym<<std::endl;
								}*/
				if(sizeone==setone.size())
				{
					//means no extra trnasition was added., just continue;;
					//see here because of copying. no element was added to the set as well..
				}else
				{
					transmap.erase(t.first);
					transmap.insert(std::make_pair(t.first,setone));
					ischanged=true;
				}
				//get transition set for t.second
			}

	}
	std::cout<<"Closure complete"<<std::endl;
	/*///Now add self loops on symbols based on if the fv of the formulas have left side of the assignment statements or not..
	std::map<AFAStatePtr,std::set<z3::expr,mapexpcomparator>,mapstatecomparator> freevarsmap;
	BOOST_FOREACH(auto t, mAllStates){
		std::set<z3::expr,mapexpcomparator> expset = GetFreeVars(t.first->mAMap);
		freevarsmap.insert(std::make_pair(t.first,expset));
	}


		BOOST_FOREACH(auto v, SeenMap){
			AutAFACmb* tmpp = v.first;
			struct autstate* nfastate=tmpp->first;
			SetAFAStatesPtr setstates = tmpp->second;
			std::set<z3::expr,mapexpcomparator> freevars;
			BOOST_FOREACH(auto w, setstates)
			{
				BOOST_ASSERT_MSG(freevarsmap.find(w)!=freevarsmap.end(),"Some serious error lookinto it carefully");
				std::set<z3::expr,mapexpcomparator> rs =freevarsmap.find(w)->second;
				freevars.insert(rs.begin(),rs.end());
			}

			BOOST_FOREACH(auto t,  mProgram.mRWLHRHMap)
				{
				   std::string sym = t.first;
				   z3::expr lhs = std::get<0>(t.second);
			 	   if(freevars.find(lhs)==freevars.end())//means not found as a free variable..
					{
						//add a self loop on this symbol to itself..
						myTransition tr;
						tr.to=nfastate;
						tr.csym=sym.at(0);
						std::set<myTransition,transcomp> stset;
						if(transmap.find(nfastate)==transmap.end()){//possible for end states..
						}else{
						stset=transmap.find(nfastate)->second;
						transmap.erase(nfastate);
						}
						stset.insert(tr);
						transmap.insert(std::make_pair(nfastate,stset));
 					}
				}

			BOOST_FOREACH(auto t,  mProgram.mCASLHRHMap)
				{
					std::string sym = t.first;

					z3::expr lhs = std::get<0>(t.second);
					if(freevars.find(lhs)==freevars.end())//means not found as a free variable..
					{
						//add a self loop on this symbol to itself..
						myTransition tr;
						tr.to=nfastate;
						tr.csym=sym.at(0);
						std::set<myTransition,transcomp> stset;
						if(transmap.find(nfastate)==transmap.end()){//possible for end states..
						}else{
						stset=transmap.find(nfastate)->second;
						transmap.erase(nfastate);
						}
						stset.insert(tr);
						transmap.insert(std::make_pair(nfastate,stset));
					}
				}
	}



*/


	//now add these transitions actually on automaton..
	BOOST_FOREACH(auto kv, transmap)
	{
		struct autstate* src = kv.first;
		std::set<myTransition,transcomp> trans = kv.second;
		BOOST_FOREACH(auto tr, trans)
		{
			add_new_auttrans(src, tr.to,tr.csym,tr.csym);
		}
	}

	std::cout<<"Transitions added along with epsilon closure if any"<<std::endl;
					//after we are done.. call delete on every key element of the map seenmap
					//no need to delete value as the key is same as the value.
					BOOST_FOREACH(auto d, SeenMap)
					{
						delete d.first;
					}

					merged->deterministic=0;
					merged->minimal=0;
					//fa_minimize(merged);
					int rescall = reverseInPlace(merged);
					BOOST_ASSERT_MSG(rescall!=-1,"Some problem in FA library in reversal operation");
					merged->deterministic=0;
					merged->minimal=0;
					fa_minimize(merged);
#ifdef DEBUG_FILE
					//minimize automaton and then return it.
					FILE *fp;
					fp = fopen("./afa2nfa.dot", "w");
					BOOST_ASSERT_MSG(fp!=NULL,"Can't open output file for wriitng afa2nfa!\n");
					fa_dot(fp,merged);
					fclose(fp);
#endif


		return merged;

 }












std::set<z3::expr,mapexpcomparator> AFAGen::GetFreeVars(z3::expr& phi){
	if(phi.is_int()&&!phi.is_numeral()){
		//BOOST_ASSERT_MSG(phi.is_var(),"Some problem: This must be var only");
		std::set<z3::expr,mapexpcomparator> res;
		res.insert(phi);
		return res;
	}else if(phi.is_app()){
		unsigned num = phi.num_args();
		std::set<z3::expr,mapexpcomparator> res;
		for (unsigned i = 0; i < num; i++) {
			z3::expr arg = phi.arg(i);
	        std::set<z3::expr,mapexpcomparator> curr= GetFreeVars(arg);
	        res.insert(curr.begin(),curr.end());
	     }
		return res;
	}
	else if(phi.is_quantifier()){
		z3::expr arg = phi.body();
		return GetFreeVars(arg);
	}else {//means phi is a constant and hence no need to cover it in the set of free variables.
		std::set<z3::expr,mapexpcomparator> res;
		return res;
	}


}


AFAStatePtr AFAGen::AddIfNotThere(z3::expr& phi,std::string& mRWord,bool& isPresent){
//create appropriate state based on the type of phi..also check if it is already present or not..
	if(phi.decl().decl_kind()==Z3_OP_AND){
		AFAStatePtr st = new ConjANDState(mRWord,phi,*this);
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
		AFAStatePtr st = new DisjANDState(mRWord,phi,*this);
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
		AFAStatePtr st = new LitORState(mRWord,phi,*this);
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

AFAGen::~AFAGen() {
 //call destructor on every state stored in mAllStates
	BOOST_FOREACH(auto t, mAllStates)
		{
		delete (t.first);
		}
}

