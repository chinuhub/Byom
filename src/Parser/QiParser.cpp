/*
 * QiParser.h
 *
 *  Created on: 13-Jun-2015
 *      Author: jaganmohini
 */

#include "QiParser.h"
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <c++/z3++.h>
// Adapted from qi spirit example of http://www.boost.org/doc/libs/1_41_0/libs/spirit/example/qi/calc2_ast.cpp
namespace client
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    using namespace z3;
    ///////////////////////////////////////////////////////////////////////////
    //  Our AST
    ///////////////////////////////////////////////////////////////////////////
    struct binary_op;
    struct unary_op;
    struct nil {};

    struct expression_ast
    {
        typedef
            boost::variant<
                nil // can't happen!
              , unsigned int
			  , std::string
              , boost::recursive_wrapper<expression_ast>
              , boost::recursive_wrapper<binary_op>
              , boost::recursive_wrapper<unary_op>
            >
        type;

        expression_ast()
          : expr(nil()) {}

        template <typename Expr>
        expression_ast(Expr const& expr)
          : expr(expr) {}

        type expr;
    };

    struct binary_op
    {
        binary_op(
            std::string op
          , expression_ast const& left
          , expression_ast const& right)
        : op(op), left(left), right(right) {}

        std::string op;
        expression_ast left;
        expression_ast right;
    };

    struct unary_op
    {
        unary_op(
            std::string op
          , expression_ast const& subject)
        : op(op), subject(subject) {}

        std::string op;
        expression_ast subject;
    };



    void unexp(expression_ast& expr1, expression_ast const& expr2, std::string t)
    {
       	expr1= expression_ast(unary_op(t, expr2));
    }

    void binexp(expression_ast& expr1, expression_ast const& expr2,expression_ast const& expr3, std::string   t)
    {
    	expr1= expression_ast(binary_op(t, expr2,expr3));
    	//std::cout<<"matched till now"<<std::endl;
    }
    ///////////////////////////////////////////////////////////////////////////
    //  Walk the tree
    ///////////////////////////////////////////////////////////////////////////
    struct eval_z3_exp
    {
        typedef z3::expr result_type;

        std::map<std::string, z3::expr>& mVarExprMap;
        z3::context& mCtx;
        eval_z3_exp(std::map<std::string, z3::expr>& VarExprMap, z3::context& c): mVarExprMap(VarExprMap),mCtx(c){}

        expr operator()(qi::info::nil_) const {BOOST_ASSERT_MSG(false,"Should not reach here");}
        expr operator()(int n) const {  return mCtx.int_val(n);}
        expr operator()(std::string const& str) const {
        	BOOST_ASSERT_MSG(mVarExprMap.find(str)!=mVarExprMap.end(),"Unexpected error: No expr found for this variable");
        	z3::expr t =mVarExprMap.find(str)->second;
        	return (t);
        }
        expr operator()(expression_ast const& ast) const
        {

            return boost::apply_visitor(*this, ast.expr);
        }

        expr operator()(binary_op const& binexp) const
        {
             //std::cout << "op:" << binexp.op << "("<<std::endl;
            z3::expr one = boost::apply_visitor(*this, binexp.left.expr);
            //std::cout << ", "<<std::endl;
            z3::expr two = boost::apply_visitor(*this, binexp.right.expr);
            /*std::cout<<"checking "<<one<<" and "<<two<<std::endl;
            std::cout<<"checking "<<one<<" op:"<<ch<<" "<<two<<std::endl;*/
            //std::cout << ')'<<std::endl;
            if(binexp.op.compare("+")==0)
            	return one+two;
            else if(binexp.op.compare("-")==0)
            	return one-two;
            else if(binexp.op.compare("*")==0)
            	return one*two;
            else if(binexp.op.compare("/")==0)
            	return one/two;
            else if(binexp.op.compare("<")==0)
                 return one<two;
            else if(binexp.op.compare(">")==0)
                 return one>two;
            else if(binexp.op.compare("<=")==0)
                 return one<=two;
            else if(binexp.op.compare(">=")==0)
            	 return one>=two;
            else if(binexp.op.compare("=")==0)
                 return (one)==(two);
            else if(binexp.op.compare("&")==0)
            	return (one)&&(two);
            else if(binexp.op.compare("|")==0)
                return (one)||(two);
            else
            	BOOST_ASSERT_MSG(false,"No proper binary operation was found in the formula");
        }

        expr operator()(unary_op const& unexp) const
        {
            //std::cout << "op:" << unexp.op << "("<<std::endl;
            z3::expr one = boost::apply_visitor(*this, unexp.subject.expr);
            //std::cout << unexp.op<<":"<<one<<std::endl;;
            if(unexp.op.compare("~")==0)
            	return !one;
            else
            {
#ifdef	DBGPRNT
            	std::cout<<"op found is "<<unexp.op<<std::endl;
#endif
            	BOOST_ASSERT_MSG(false,"No proper unary operation was found in the formula");
            }


        }
    };

    ///////////////////////////////////////////////////////////////////////////
    //  Our calculator grammar
    ///////////////////////////////////////////////////////////////////////////
    //Some learning points about spirit grammar
    //for long time i was struggling to find out how to change the synth attribute of a rule (_val)
    //easiest way is now done here.. create a function and call it lazily in the semantic action
    //pass _val also as an argument (by referenc) that will allow you to change it inside dpeendiing upon your need
    //you can pass other synthesized attributes of parameters of a rule.. like _1, _2 etc..
    //make sure that while doing so  you put the while rule (part of |) inside a parenthesis only then _2, _3 will
    //be properly formed.. otherwise you will get error that vector is accesing out of indexed element.
    template <typename Iterator>
    struct expparser : qi::grammar<Iterator, expression_ast(),ascii::space_type>
    {
        expparser() : expparser::base_type(top)
        {
            using qi::_val;
            using qi::_1;
            using qi::_2;
            using qi::_3;
            using qi::uint_;
            using qi::lit;
            using qi::alnum;

            top = bexp | aexp;

            bexp= (singlebexp[_val=_1] >> +(lit("&&") >> bexp)[boost::phoenix::bind(binexp,_val,_val,_1,"&")])
						| (singlebexp[_val=_1] >> +(lit("||") >> bexp)[boost::phoenix::bind(binexp,_val,_val,_1,"|")])
            			| singlebexp;

           singlebexp = (aexp >> lit("<") >> aexp)[boost::phoenix::bind(binexp,_val,_1,_2,"<")]
						| (aexp >> lit(">") >> aexp)[boost::phoenix::bind(binexp,_val,_1,_2,">")]
						| (aexp >> lit("<=") >> aexp)[boost::phoenix::bind(binexp,_val,_1,_2,"<=")]
						| (aexp >> lit(">=") >> aexp)[boost::phoenix::bind(binexp,_val,_1,_2,">=")]
						| (aexp >> lit("==") >> aexp)[boost::phoenix::bind(binexp,_val,_1,_2,"=")]
           	   	   	   	| (lit("~") >> bexp)[boost::phoenix::bind(unexp,_val,_1,"~")]
           				| (lit("(") >> bexp >> lit(")"))[_val = _1]
						| (lit("(") >> singlebexp >> lit(")"))[_val = _1];


            aexp = (tm[_val=_1] >> +(lit("+") >> aexp)[boost::phoenix::bind(binexp,_val,_val,_1,"+")])
            	| (tm[_val=_1] >>  +(lit("-") >> aexp)[boost::phoenix::bind(binexp,_val,_val,_1,"-")])
            	| (tm[_val=_1] >>  +(lit("*") >> aexp)[boost::phoenix::bind(binexp,_val,_val,_1,"*")])
            	| (tm[_val=_1] >>  +(lit("/") >> aexp)[boost::phoenix::bind(binexp,_val,_val,_1,"/")])
				| (lit("(") >> aexp >> lit(")"))[_val = _1]
                | tm;

            tm = (nums | term)[_val=_1];
            nums = uint_                           [_val = _1];
            term = qi::as_string[qi::alpha >> *qi::alnum];
        }

        qi::rule<Iterator, expression_ast(),ascii::space_type  >
        nums,aexp,bexp,tm,term,singlebexp,top;
    };

  Parser* GetParser(){ return new Parser();}

  z3::expr ParseExp(Parser& parser,std::string& input,std::map<std::string, z3::expr>& VarExprMap, z3::context& ctx)
  {
  	 using boost::spirit::ascii::space;
  	 using client::expression_ast;
  	 using client::eval_z3_exp;

     std::string::const_iterator beg = input.begin();
  	 std::string::const_iterator end = input.end();
  	 expression_ast ast;
  	 eval_z3_exp evaluator(VarExprMap,ctx);
  	 bool r = boost::spirit::qi::phrase_parse(beg, end, parser, space, ast);
  	 if(!r || beg!=end)
  	 {
  		 std::string::iterator it = input.begin();
  		 while(it!=beg+1)
  		 {
  			 std::cout<<*it;
  			 it++;
  		 }
  		 std::cout<<std::endl;
  		std::cout<<"Failed for string "<<input<<std::endl;
  		 BOOST_ASSERT_MSG(false,(std::string("not yet")+input).c_str());
  	 }else
  	 {
  	  		 return evaluator(ast);
  	 }
  }
}





