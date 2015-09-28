/*
 * AFAGen.h
 *
 *  Created on: 15-Jun-2015
 *      Author: jaganmohini
 */

#ifndef PROOFAFA_AFAGEN_H_
#define PROOFAFA_AFAGEN_H_
#include"../Parser/Program.h"
#include<memory>
#include<map>
#include<set>
#include<vector>
#include<unordered_map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
extern "C"
{
#include <fa.h>
}

#include<c++/z3++.h>
class AFAState;//forwards declaration
typedef AFAState* AFAStatePtr;
//typedef to refer a set of states with ease...

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


struct mapstatecomparator{
	bool operator() (const AFAStatePtr& one, const AFAStatePtr& two) const;
};
typedef std::set<AFAStatePtr,mapstatecomparator> SetAFAStatesPtr;

struct mapexpcomparator{
	bool operator() (const z3::expr& one, const z3::expr& two) const;
};

class AFAGen {




	//set of accepted states
	std::string& mWord;
	z3::expr& mPhi;
	//Construction Pass for AFA construction
		void PassOne(AFAStatePtr& state);
		//Pass to collapse collapsible edges
		void PassTwo(AFAStatePtr& state, std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator>& seenStates);
		//Pass to Print DOT file
		void PassThree(AFAStatePtr& state, Graph& g, std::map<AFAStatePtr, vertex_t, mapstatecomparator>& indmap);
		//Pass to add more edges..
		void PassFour(Graph& g, std::map<AFAStatePtr,vertex_t,mapstatecomparator>& mapindex);
		//function to conver this AFA to NFA and return
		struct fa* ConvertToNFA(AFAStatePtr);

		std::vector<SetAFAStatesPtr> getValues(std::multimap<std::string, SetAFAStatesPtr>& map, std::string& key);
public:
		//set of states as map, this will be filled incrementatlly while constrution and will also be used as
			//set of states seen so far.
			std::map<AFAStatePtr,AFAStatePtr,mapstatecomparator> mAllStates;
	Program& mProgram;
	//initial state
		AFAStatePtr init;

	AFAGen(std::string& w, z3::expr& phi,Program& p);
	struct fa* ProcessProof(int);
	//add to SeenSet if not.. else return the pointer..
	AFAStatePtr AddIfNotThere(z3::expr& phi,std::string& mRWord,bool& isPresent);
	//function to get free variables present in expression phi
	std::set<z3::expr,mapexpcomparator> GetFreeVars(z3::expr& phi);


	virtual ~AFAGen();
};

#endif /* PROOFAFA_AFAGEN_H_ */
