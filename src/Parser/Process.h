/*
 * Process.h
 *
 *  Created on: 10-Jun-2015
 *      Author: jaganmohini
 */

#ifndef PARSER_PROCESS_H_
#define PARSER_PROCESS_H_
#include <iostream>
#include <vector>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <unordered_map>
#include <tuple>
#include "Program.h"
#include <c++/z3++.h>
#include "QiParser.h"
class Process {
	Program& mProgram;
	z3::context& mCtx;
	client::Parser& mParser;
public:
	Process(Program& P, z3::context& c, client::Parser& par);

	void ParseInput(std::ifstream& );
	virtual ~Process();
};

#endif /* PARSER_PROCESS_H_ */
