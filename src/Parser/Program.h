/*
 * Program.h
 *
 *  Created on: 09-Jun-2015
 *      Author: Chinmay Narayan
 *      Description: This class is responsible for parsing the input file and generate
 *      an appropriate number of Process objects, one for each process description present in
 *      the file. These process objects are stored in an object of this class.
 */

#ifndef PARSER_PROGRAM_H_
#define PARSER_PROGRAM_H_
#include<iostream>
#include<fstream>
#include<vector>
#include <unordered_map>
#include<map>
#include "../ResultType.h"
#include "QiParser.h"
#include <c++/z3++.h>
#include <tuple>
struct z3comparator{
	bool operator()(const z3::expr& one, const z3::expr& two) const;
};
class Program {
	std::string* mFileName;
	std::map<std::string, z3::expr> mGlobalVars;//map for global variable and its initial values
	client::Parser& mParser;

public:
	std::string mInitString;
	z3::context& mCtx;
	std::map<std::string, z3::expr> mVarExprMap;
	std::map<std::string, std::tuple<z3::expr, z3::expr>> mRWLHRHMap;
	std::map<std::string, z3::expr> mAssumeLHRHMap;
	std::map<z3::expr,std::string,z3comparator> mRevAssumeLHRHMap;
	std::map<std::string, std::tuple<z3::expr, z3::expr,z3::expr>> mCASLHRHMap;
	std::map<std::string, z3::expr> mAssnMap;//map for label-> assert statements for all processes
	std::vector<std::string> mProcessesregex;
	std::vector<std::string> mAllSyms;
	Program(const char* filename, z3::context& c, client::Parser& parser);
	ResultType ProcessInput();
	z3::expr AddIfNot(std::string& var);
	std::vector<std::string> GetRegexOfAllProcesses();
	std::map<std::string, z3::expr >& GetAssnMapForAllProcesses();
	void ExtractGlobalVarsAssignment(std::string line);
	void PrintGlobalVars();
	void PrintVector( std::vector <std::string> & v );
	virtual ~Program();
};

#endif /* PARSER_PROGRAM_H_ */
