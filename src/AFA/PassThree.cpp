/*
 * PassThree.cpp
 *
 *  Created on: 25-Aug-2015
 *      Author: jaganmohini
 */
#include "AFAState.h"
#include<boost/assert.hpp>


void AFAState::PassThree(Graph& g, std::map<AFAStatePtr, vertex_t,mapstatecomparator>& indmap){//imp not to have mapstatecomparator here just compare raw pointers
	if(indmap.find(this)!=indmap.end())
		return;
	vertex_t vthis = boost::add_vertex(g);
	indmap.insert(std::make_pair(this,vthis));

	if(mType==AND){
		SetAFAStatesPtr nextset;
		//create a new vertes for this state..

		VertexProp vp;
		std::stringstream stream;
		stream << mAMap<<","<<(*mHMap)<<","<this;//<mRWord;
		vp.vertlabel=stream.str();
		stream.str(std::string());//erase the buffer
		stream<<((*mHMap));
		vp.xlabel=stream.str();
		vp.shape="rectangle";
		vp.color="yellow";
		g[vthis]=vp;
		int count=0;
//		std::cout<<"Total number of transition sout from this stat is "<<mTransitions.size()<<std::endl;
		BOOST_FOREACH(auto t, mTransitions)
		{
			nextset=t.second;
			std::string sym = t.first;
			EdgeProp edgep;
			edgep.label=sym.append((std::to_string(count)).c_str());
			BOOST_FOREACH(auto st, nextset)
			{
				st->PassThree(g,indmap);
				BOOST_ASSERT_MSG(indmap.find(st)!=indmap.end(),"Some problem as by now this must have been added");
				vertex_t vert = indmap.find(st)->second;
				if(st!=this)
				BOOST_ASSERT_MSG(vert!=vthis,"Some serious issue");
				//creat a dot transitions from index to dotnum to the file..
				edge_t e;
				bool added;
				boost::tie(e,added)=boost::add_edge(vthis,vert,g);
				BOOST_ASSERT_MSG(added,"Some issue");
				g[e]=edgep;
			}
			count++;
		}


	}else if (mType==OR){
		SetAFAStatesPtr nextset;
		//create a new vertes for this state..

		VertexProp vp;
		std::stringstream stream;
		stream << mAMap<<","<<(*mHMap)<<","<<this;//mRWord;
		vp.vertlabel=stream.str();
		stream.str(std::string());//erase the buffer
		stream<<((*mHMap));
		vp.xlabel=stream.str();
		vp.shape="rectangle";
		if(mIsAccepted){
			vp.shape="doubleoctagon";
		}
		vp.color="cyan";
		g[vthis]=vp;
		int count=0;
//		std::cout<<"Total number of transition sout from this"<< this<<" stat is "<<mTransitions.size()<<std::endl;
		BOOST_FOREACH(auto t, mTransitions)
			{
				nextset=t.second;
				std::string sym = t.first;
				EdgeProp edgep;
				edgep.label=sym+std::to_string(count);
				BOOST_FOREACH(auto st, nextset)
				{
//					std::cout<<"Transition to "<<st<<" from "<<this<<" on sym"<<std::endl;
					st->PassThree(g,indmap);
					BOOST_ASSERT_MSG(indmap.find(st)!=indmap.end(),"Some problem as by now this must have been added");
					vertex_t vert = indmap.find(st)->second;
					if(st!=this)
					BOOST_ASSERT_MSG(vert!=vthis,"Some serious issue");

					//creat a dot transitions from index to dotnum to the file..
					edge_t e;
					bool added;
					boost::tie(e,added)=boost::add_edge(vthis,vert,g);
					BOOST_ASSERT_MSG(added,"Some issue");
					g[e]=edgep;

				}
				count++;
			}

	}else if(mType==ORLit){
		SetAFAStatesPtr nextset;
		//create a new vertes for this state..

		VertexProp vp;
		std::stringstream stream;
		stream << mAMap<<","<<(*mHMap)<<","<<this;//mRWord;
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
		int count=0;

//		std::cout<<"Total number of transition sout from this stat is "<<mTransitions.size()<<std::endl;
		BOOST_FOREACH(auto t, mTransitions)
		{
				nextset=t.second;
				std::string sym = t.first;
				EdgeProp edgep;
				edgep.label=sym+std::to_string(count);
				BOOST_FOREACH(auto st, nextset)
				{
					st->PassThree(g,indmap);
					BOOST_ASSERT_MSG(indmap.find(st)!=indmap.end(),"Some problem as by now this must have been added");
					vertex_t vert = indmap.find(st)->second;
					if(st!=this)
						BOOST_ASSERT_MSG(vert!=vthis,"Some serious issue");

					//creat a dot transitions from index to dotnum to the file..
					edge_t e;
					bool added;
					boost::tie(e,added)=boost::add_edge(vthis,vert,g);
					BOOST_ASSERT_MSG(added,"Some issue");
					g[e]=edgep;
				}
				count++;
		}

	}else
		BOOST_ASSERT_MSG(false,"Serious error, control must not have reached here");
}
