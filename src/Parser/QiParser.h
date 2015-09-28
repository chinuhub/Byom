/*
 * QiParser.h
 *
 *  Created on: 13-Jun-2015
 *      Author: jaganmohini
 */

#ifndef PARSER_QIPARSER_H_
#define PARSER_QIPARSER_H_
#include<iostream>
#include<c++/z3++.h>
#include <map>
namespace client{

template <typename Iterator> struct expparser;
typedef expparser<std::string::const_iterator> Parser;
Parser* GetParser();
z3::expr ParseExp(Parser& parser, std::string& input, std::map<std::string, z3::expr>& , z3::context& );
}


#endif /* PARSER_QIPARSER_H_ */
