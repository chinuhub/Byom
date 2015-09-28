/*
 * Test.cpp
 *
 *  Created on: 09-Jun-2015
 *      Author: jaganmohini
 */
#include "AFA/AFAut.h"
#include "Parser/Program.h"
#include "SCTransSystem/SCTransSystem.h"
#include "Parser/QiParser.h"
#include <boost/chrono.hpp>
#include <boost/assert.hpp>
#include <iostream>
#include <tuple>
#include <string>
#include <algorithm>
#define DEBUGSTMTS
#include <c++/z3++.h>
bool checkAccepting(std::string& word, struct fa* aut);
struct autstate* getNext(struct autstate* state, char c);
int main(int argc, char* argv[]){

	if(argc==1){
		std::cout<<"Enter the program file as first argument"<<std::endl;
		exit(-1);
	}
	std::string filename(argv[1]);

	auto start =  boost::chrono::system_clock::now();
	z3::context ctx;
	z3::solver s(ctx);
	client::Parser* expParser = client::GetParser();//it is callers responsibility to delete the parser object.
	Program* P = new Program(filename.c_str(),ctx,*expParser);
	P->ProcessInput();
	SCTransSystem* T = new SCTransSystem(*P,s);
	struct fa* revmerged = T->BuildSCTS();//it has initial values as well with them..This will construct reverse of shuffle


	char* word;
	size_t length;
	fa_example(revmerged,&word,&length);
	std::string revword(word,length);

	int cases=0;
	while(revword.length()!=0)
	{

		std::string original(revword);//for some debugging purposes
		std::reverse(revword.begin(), revword.end());//reverse the word to get it back.
		std::cout<<"Getting accepted state for "<<revword<<std::endl;
		z3::expr wordphi= T->GetEndStateAssertionFromWord(revword);
		std::string exword = revword;
		std::cout<<"One accepted word is "<<exword<<" with phi = "<<wordphi<<std::endl;

		z3::expr negphi = !wordphi;
		AFAut* proofafa = AFAut::MakeAFAutProof(exword,negphi,P,cases);
		std::cout<<"AFA construction over"<<std::endl;
		if(proofafa->IsUnsafe())
		{
			std::cout<<"An errorneous trace "<<std::endl;
			std::cout<<exword<<std::endl;
			std::exit(-1);
		}


#ifdef	DEBUGSTMTS
		bool res =  proofafa->IsAccepted(original);///This debugging function was necessary , how to implemeent it.??
		BOOST_ASSERT_MSG(res==true,"Some problem in afa construction as this word must be present there");
#endif

		proofafa->Complement();
		std::cout<<"Complemeent over"<<std::endl;

#ifdef	DEBUGSTMTS
		proofafa->PrintToDot("DOTcomplement.dot");
#endif
		struct fa* complementedfa = proofafa->ConvertToNFA();
		delete proofafa;
		std::cout<<"Conversion to nfa over"<<std::endl;
#ifdef	DEBUGSTMTS
		fa_minimize(complementedfa);
		std::cout<<"minimization of nfa over"<<std::endl;
#endif
		struct fa* tmp=fa_intersect(complementedfa,revmerged);
		fa_free(revmerged);
		revmerged=tmp;
		std::cout<<"Interesection over "<<std::endl;
		/*revmerged->deterministic=0;
		revmerged->minimal=0;
		fa_minimize(revmerged);*/
		//std::cout<<"minimization of interesected nfa over"<<std::endl;
		/*res =  revmergedafa->IsAccepted(tt);///This debugging function was necessary , how to implemeent it.??
			if(res)
				std::cout<<"Accepted by intersection as well"<<cases+1<<std::endl;
		*/
		//fa_example_withendstate(revmerged,&word,&length,&accstate);
		fa_example(revmerged,&word,&length);
		revword=std::string(word,length);

		//std::cout<<"New word is "<<revword<<std::endl;
		//std::exit(-1);
		int i;
		//std::cout<<"Accepted word length is "<<revword.length()<<std::endl;
		std::cin>>i;
		cases++;

	}
	std::cout<<"Total cases = "<<cases<<std::endl;
	delete P;
	delete T;
	delete expParser;
	auto end = boost::chrono::system_clock::now();
	auto   elapsed = boost::chrono::duration_cast<boost::chrono::duration<double> >(end- start).count();
	std::cout << "Time spent = "<<elapsed << "seconds "<<'\n';
	return 0;

}

//TODO:
//1. AFA subtraction without construction NFA inbetween..
//2. running benchmarks on SVComp
bool checkAccepting(std::string& word, struct fa* aut)
{
	std::cout<<"Checking if "<<word<<" is still accepted by the returned automaton"<<std::endl;
	struct autstate* reached=aut->initial;
	for(std::string::iterator it = word.begin(); it!=word.end(); it++)
	{
		char c = (*it);
		reached = getNext(reached,c);
	}
	if(reached->accept==1)
		return true;
	else return false;
}

struct autstate* getNext(struct autstate* state, char c){

	int i=0;
#if DEBUG_LEVEL==1
	std::cout<<"Checking transition on "<<c<<" from state "<<state<<std::endl;
#endif
	while(i<state->tused){
		if((state->trans+i)->min <= c && (state->trans+i)->max >= c)
			return (state->trans+i)->to;
		i++;
	}
	BOOST_ASSERT_MSG(false, "Something wrong happened, did not found any transition on thi scharacter from this state");
}


