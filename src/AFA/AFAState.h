/*
 * AFAState.h
 *
 *  Created on: 23-Aug-2015
 *      Author: jaganmohini
 */

#ifndef AFA_AFASTATE_H_
#define AFA_AFASTATE_H_
#include<set>
#include<boost/graph/adjacency_list.hpp>
#include<boost/graph/graphviz.hpp>
#include<map>
#include<tuple>
#include<c++/z3++.h>
extern "C"
{
#include <fa.h>
}

#include <libfaudes.h>
//Define a class that has the data you want to associate to every vertex and
  //edge
  struct EdgeProp{std::string label;};
  struct VertexProp{ std::string vertlabel; std::string xlabel; std::string shape; std::string color;};
  //
  ////Define the graph using those classes
  typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, VertexProp, EdgeProp > Graph;

  ////Some typedefs for simplicity
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
  typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

  class AFAState;
  typedef AFAState* AFAStatePtr;
  struct mapstatecomparator{
  	bool operator() (const AFAStatePtr& one, const AFAStatePtr& two) const;
  };

  struct mapexpcomparator{
  	bool operator() (const z3::expr& one, const z3::expr& two) const;
  };

  typedef std::set<AFAStatePtr,mapstatecomparator> SetAFAStatesPtr;

  struct transitionscomparator{
	  bool operator() (const std::pair<std::string,SetAFAStatesPtr>& one, const std::pair<std::string,SetAFAStatesPtr>& two) const;
    };

  struct transitionscomparatorraw{
	  bool operator() (const std::pair<std::string,std::set<AFAStatePtr>>& one, const std::pair<std::string,std::set<AFAStatePtr>>& two) const;
  };

  struct tuplecomparator{
  	bool operator() (const std::tuple<AFAStatePtr,std::string,AFAStatePtr>& one, const std::tuple<AFAStatePtr,std::string,AFAStatePtr>& two) const;
  };

  struct mymapstatecomparator{
  	bool operator() (const AFAStatePtr& one, const AFAStatePtr& two) const;
  };

  typedef std::map<struct autstate*, std::tuple<std::string,std::string,std::map<std::string,std::set<struct autstate*>>>> autstateinfotype;
enum StateType {AND,OR,ORLit};
class AFAState {


public:
	std::map<z3::expr, bool,mapexpcomparator> mUnsatMemoization;
	std::string mAssumeSym;
	StateType mType;
	//set of transitions going out of this state..
	std::multimap<std::string,SetAFAStatesPtr> mTransitions;
	//flag to check if this is an accepting state or not
	bool mIsAccepted;
	//Assn associated with this state
	z3::expr mAMap;
	//residual string
	std::string mRWord;
	//Hoare assn associated with this state
	//NOte that we cant use rference here because then we wont be able to set it later,,
	//adn we cant use normal var becaus then it is like o argument constructor which odes not exist for z3::expr
	//and hence give error.. Hence w have to live with * declaration., or rathe rmake it shared_ptr;;
	z3::expr* mHMap;
	AFAState(StateType type, std::string& wrd, z3::expr& phi):mRWord(wrd), mAMap(phi)
	{mType=type; mIsAccepted=false;mHMap=NULL; mAssumeSym="";
	}
	AFAState(StateType type, z3::expr& phi): mAMap(phi)
	{
		mType=type;mIsAccepted=false,mHMap=new z3::expr(phi);
		std::stringstream st;
		st<<this;
		mRWord=st.str();
		mAssumeSym="";
	}
	void PassOne(std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& mAllStates);
	bool PassTwo(std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& mAllStates);

	//Pass to Print DOT file
	void PassThree(Graph& g, std::map<AFAStatePtr, vertex_t,mapstatecomparator>& indmap);

	std::set<std::tuple<std::string,AFAStatePtr>>  PassFourPhaseOne(std::set<AFAStatePtr>& ANDORStates, std::set<AFAStatePtr>& ORLitStates,std::set<std::tuple<AFAStatePtr,std::string,AFAStatePtr>, tuplecomparator>& toANDLink,std::map<AFAStatePtr,std::set<std::tuple<std::string,AFAStatePtr>>>& );
	std::set<std::set<AFAStatePtr>> PassFourPhaseZero(std::map<AFAStatePtr,std::set<std::set<AFAStatePtr>>>&);

	//struct fa* PassFour(AFAStatePtr init, std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& allStates );

		//std::string getOredString(std::string& in);
	struct fa* createRepeat(std::string &in);
	struct fa* createOr(std::string &in);
//BEWARE: having mapstatecomparator might be problematic if states do not have rword/amap in them like those created manually.
	//bool HelperAddEdgeIfAbsent(AFAStatePtr src,AFAStatePtr dest,std::string sym,Graph& g,std::map<AFAStatePtr, vertex_t,mapstatecomparator>mapindex);
	bool HelperAddEdgeIfAbsent(AFAStatePtr src,AFAStatePtr dest,std::string sym);
	AFAStatePtr HelperAddStateIfAbsent(z3::expr& phi,std::string& mRWord,bool& isPresent, std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& mAllStates);
	std::set<z3::expr,mapexpcomparator> HelperGetFreeVars(z3::expr& phi);
	z3::expr HelperSimplifyExpr(z3::expr exp);
	bool HelperIsUnsat(z3::expr formula);
	bool HelperIsValid(z3::expr formula);

	std::set<std::string> getTransitionKeys();

	virtual ~AFAState();
};



#endif /* AFA_AFASTATE_H_ */
