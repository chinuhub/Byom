/*
 * Program.cpp

 *
 *  Created on: 09-Jun-2015
 *      Author: Chinmay Narayan
 */

#include "Program.h"
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/algorithm/string/regex.hpp>
#include "Process.h"

bool z3comparator::operator()(const z3::expr& one, const z3::expr& two) const
{
	return one.hash()<two.hash();
}


	Program::Program(const char* filename, z3::context& c, client::Parser& parser):mCtx(c),mParser(parser){
		mFileName = new std::string(filename);

	}

	/**
	 * A big function to do the work of parsing, whenever it sees a line in the input file
	 * that denotes a new process, it creates a new process object and pass the responsibility
	 * of parsing the relevant information to the class Process. By this any change at the process
	 * description level gets localized to the class Process.
	 */
	ResultType Program::ProcessInput(){
		std::ifstream fin;
		std::string line;
		fin.open(mFileName->c_str());
#ifdef	DBGPRNT
		std::cout<<"About to open the file "<<mFileName->c_str()<<std::endl;
#endif
		if(!fin.is_open())
		{
			BOOST_ASSERT_MSG(false,"Not able to open the input file");
		}
			/**
			 * Extract global variables and store in a map
			 */
			std::getline(fin,line);
			if(line.length()!=0)
			{
				std::vector <std::string> fields;
				boost::split_regex( fields, line, boost::regex( ":" ) );
				BOOST_ASSERT_MSG(fields.front().compare("Init")==0, "First line of the input file must start with Init for global variables");
				fields.erase(fields.begin());//remove the first element Init
				line=fields.front();
				ExtractGlobalVarsAssignment(line);
			}
			/**
			 * Extract process information and pass the filepointer to Process clss to fill their object.
			 */
			while(fin.good())
			{
				std::getline(fin,line);
				if(line.length()==0)
					break;
				std::vector <std::string> fields;
				boost::split_regex( fields, line, boost::regex( ":" ) );
				//std::cout<<"instead of proce we found "<<fields.front()<<std::endl;
				BOOST_ASSERT_MSG(fields.front().compare("Proc")==0, "Process description should start with Proc keyword");
				//Pass the fin object to a newly created Process class.
				Process proc(*this,mCtx,mParser);
				proc.ParseInput(fin);
			}

		fin.close();
		return Pro_Parse_Succ;
	}

	std::vector<std::string> Program::GetRegexOfAllProcesses()
	{
		return mProcessesregex;
	}

	std::map<std::string, z3::expr>& Program::GetAssnMapForAllProcesses()
	{
			return mAssnMap;

	}
	void Program::ExtractGlobalVarsAssignment(std::string line)
	{
		std::vector <std::string> fields;
		boost::split_regex( fields, line, boost::regex( "," ) );
		int count =1;
		BOOST_FOREACH(std::string& strs, fields){
					std::vector<std::string> varsval;
					boost::split_regex( varsval, strs, boost::regex( "=" ) );
					//first one is variable, third one is value.
					std::string vars = varsval.front();
					std::string val(varsval.back());
					z3::expr valexp = client::ParseExp(mParser,val,mVarExprMap,mCtx);
					z3::expr varexp = AddIfNot(varsval.front());
					mGlobalVars.insert(std::make_pair(vars,valexp));
					mVarExprMap.insert(std::make_pair(vars,varexp));
					//also add them as a symbol in LRWMap..with new symbols starting from 1 onwards..
#ifdef	DBGPRNT
					std::cout<<"Inserting initial values as write "<<vars<<"="<<val<<std::endl;
#endif
					std::string symname = std::to_string(count);
					mAllSyms.push_back(symname);//so that thse symbols are also present here..
					count++;
					mInitString=mInitString+symname;
					std::pair<z3::expr,z3::expr> pr = std::make_pair(varexp,valexp);
					mRWLHRHMap.insert(std::make_pair(symname,pr));
			}
#ifdef	DBGPRNT
		PrintGlobalVars();
#endif
	}

	z3::expr Program::AddIfNot(std::string& var)
	{
		if(mVarExprMap.find(var)==mVarExprMap.end()){
			z3::expr exp= mCtx.int_const(var.c_str());
			mVarExprMap.insert(std::make_pair(var,exp));
			return exp;
		}else
			return mVarExprMap.find(var)->second;
	}


	void Program::PrintVector( std::vector <std::string> & v )
	{
	  for (size_t n = 0; n < v.size(); n++)
	    std::cout << "\"" << v[ n ] << "\"\n";
	  std::cout << std::endl;
	}

	void Program::PrintGlobalVars(){

		BOOST_FOREACH(auto pars, mGlobalVars){
			std::cout<<pars.first<<"="<<pars.second<<std::endl;
		}
	}

	/**
	 * Destructor
	 */
	Program::~Program() {
		delete mFileName;
	}
