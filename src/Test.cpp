

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
#include <libfaudes.h>
 //#define SANITYCHKASSN 0
//#define DBGPRNT 0
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
	faudes::Generator generator,lGenerator;
	z3::context ctx;
	z3::solver s(ctx);
	client::Parser* expParser = client::GetParser();//it is callers responsibility to delete the parser object.
	Program* P = new Program(filename.c_str(),ctx,*expParser);
	P->ProcessInput();
	SCTransSystem* T = new SCTransSystem(*P,s);
	struct fa* revmerged = T->BuildSCTS(lGenerator);//it has initial values as well with them..This will construct reverse of shuffle
	//it fills the generator as well..
   //lGenerator.StateNamesEnabled(false);
   faudes::Deterministic(lGenerator,generator);
   generator.DotWrite("DetermOriginal.dot");
   //std::cout<<"Determinisitic done"<<std::endl;

   //faudes::StateMin(lGenerator,generator);

   std::cout<<"Determinization done "<<std::endl;
   generator.StateNamesEnabled(false);
   faudes::aStateMin(generator);
   std::cout<<"Minimization done "<<std::endl;


	char* word;
	size_t length;
	fa_example(revmerged,&word,&length);
	std::string revword(word,length);
	int i;
	int cases=0;
  while(!faudes::IsEmptyLanguage(generator))
  {
//	while(revword.length()!=0)
//	{

    std::string revword(generator.GetWord());
		std::string original(revword);//for some debugging purposes
    std::cout<<"Original is "<<original<<std::endl;
		std::reverse(revword.begin(), revword.end());//reverse the word to get it back.
#ifdef DBGPRNT
		std::cout<<"Getting accepted state for "<<revword<<std::endl;
#endif
		z3::expr wordphi= T->GetEndStateAssertionFromWord(revword);
		std::string exword = revword;
		std::cout<<"Checking word "<<exword<<" with postcondition phi = "<<wordphi<<std::endl;

		z3::expr negphi = !wordphi;
		//struct fa* prooffa = AFAut::MakeAFAutProof(exword,negphi,P,cases);
		bool result=false;
		faudes::Generator proofgens;
		AFAut* proofafa = AFAut::MakeAFAutProof(exword,negphi,P,cases,result,proofgens);
		//what we got is already complemented version..
		if(result==false)//return null if hmap of initial state is unsat.
				{
					std::cout<<"An errorneous trace "<<std::endl;
					std::cout<<exword<<std::endl;
					std::exit(-1);
				}

#ifdef SANITYCHKASSN
		std::cout<<"AFA construction over-complement.. dumping to complemented.dot file.Press any int to continue"<<std::endl;
		bool res= checkAccepting(original,prooffa);
		BOOST_ASSERT_MSG(res==false,"Some serious error as this word must not be accepted by this complemented FA");
#endif
	 faudes::Generator intersectionres;
	 std::cout<<"Original: States="<<generator.States().Size()<<" Transitions = "<<generator.TransRel().Size()<<std::endl;
	 proofafa->Intersection(generator,intersectionres);
	 generator.Clear();
     faudes::Deterministic(intersectionres,generator);
	 //generator.Assign(intersectionres);
     intersectionres.Clear();
	 faudes::aStateMin(generator);
	 delete proofafa;
     std::cout<<"Intersection: States = "<<generator.States().Size()<<" Transitions = "<<generator.TransRel().Size()<<std::endl;

     //generator.Assign(intersectionres);

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
//2. running benchmarks on SVComp
bool checkAccepting(std::string& word, struct fa* aut)
{

	std::cout<<"Checking if "<<word<<" is still accepted by the returned automaton"<<std::endl;
	struct autstate* reached=aut->initial;
	for(std::string::iterator it = word.begin(); it!=word.end(); it++)
	{
		char c = (*it);
		reached = getNext(reached,c);
		if(reached==NULL)//means we did not have complete fa and we did not find a transition on some symbol.. hence
				return false;			//not accepted, therefore return false;
	}
	if(reached->accept==1)
		return true;
	else return false;
}

struct autstate* getNext(struct autstate* state, char c){

	int i=0;
	std::cout<<"Checking transition on "<<c<<" from state "<<state<<std::endl;
	while(i<state->tused){
		if((state->trans+i)->min <= c && (state->trans+i)->max >= c)
			return (state->trans+i)->to;
		i++;
	}
	return NULL;
	BOOST_ASSERT_MSG(false, "Something wrong happened, did not found any transition on thi scharacter from this state");
}


