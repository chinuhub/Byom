/** @file cfl_token.h @brief Class Token */

/* FAU Dscrete Event Systems Library (libfaudes)

Copyright (C) 2006  Bernd Opitz
Exclusive copyright is granted to Klaus Schmidt

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef FAUDES_TOKEN_H

#include "cfl_definitions.h"
#include "cfl_helper.h"
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>

namespace faudes {

/**
 * Tokens model atomic data for stream IO.
 *
 * A Token models a string or numeric datum that can be read from a
 * or written to a C++ stream. The class itself implements the representation of 
 * the data including its type. For section handling and actual file processing
 * see TokenReader and TokenWriter.
 *
 * @param mType
 *   faudes::TokenType of the Token
 * @param mStringValue
 *   Token value as C++-type std::string
 * @param mOptionValue
 *   Token value as interpreted option token
 * @param mIntegerValue
 *   Token value as C++-type integer, faudes typedef Int
 * @param mFloatValue
 *   Token value of C++-type double, faudes typedef Float
 *
 * @ingroup TokenIO
 */

class Token {
public:

  friend class TokenWriter;
  friend class TokenReader;

  /**
   * Empty constructor, constructs None token
   */
  Token(void);
		
  /**
   * Copy constructor
   */
  Token(const Token& rToken);


  /**
   * Token destructor
   */
  ~Token(void);

  /** Assignment operator */
  Token& operator= (const Token& rOther);

  /**
   * Token types:
   */
  enum TokenType {
    None=     0x000,  ///<  Invalid/empty token    
    Begin=    0x001,  ///<  \<label\>   (begin of section)
    End=      0x002,  ///<  \<\\label\> (end of section)
    String=   0x004,  ///<  any string, space separated or quoted
    Option=   0x008,  ///<  +xyZ+       (option string, may not contain a "+")
    Integer=  0x010,  ///<  1234        (non-negative integer)
    Integer16=0x020,  ///<  0x12fff     ("0x" makes a number Integer16) 
    Boolean=  0x040,  ///<  True/False  
    Number=   0x080,  ///<  -12.34      ("-" or "." turns an integer to a number)
    Binary=   0x100,  ///<  =ABhlkjj=   (base64 encoded binary data)
    Cdata=    0x200   ///<  <![CDATA[ ... ]]> markup
  };


  /**
   * Initialize None token
   * 
   */
  void SetNone(void);

  /**
   * Initialize as String token
   * 
   * @param rName
   *   String to fill the Token
   */
  void SetString(const std::string& rName);

  /**
   * Initialize as Begin token
   * 
   * @param rName
   *   Title of section to fill the Token
   */
  void SetBegin(const std::string& rName);

  /**
   * Initialize as End token
   * 
   * @param rName
   *   Title of section to fill the Token
   */
  void SetEnd(const std::string& rName);

  /**
   * Initialize as empty-tag token
   * 
   * @param rName
   *   Title of section to fill the Token
   */
  void SetEmpty(const std::string& rName);

  /**
   * Initialize as Option token
   * 
   * @param rName
   *   Option to fill the Token
   */
  void SetOption(const std::string& rName);

  /**
   * Initialize as Integer token
   *
   * @param number
   *   Number to fill the Token
   */
  void SetInteger(const Int number);

  /**
   * Initialize as Integer16 token
   *
   * @param number
   *   Number to fill the Token
   */
  void SetInteger16(const Int number);

  /**
   * Initialize as Boolean token 
   *
   * @param number
   *   Number to fill the Token
   */
  void SetBoolean(const Int number);

  /**
   * Initialize as Float token
   *
   * @param number
   *   Number to fill the Token
   */
  void SetFloat(const Float number);
    
  /**
   * Initialize Binary token.
   * This method allocates a copy of the data.
   * For writing only, you may use the TokenWriter interface
   * to avoid the local copy.
   *
   * @param data
   *   Reference to raw data record
   * @param len
   *   Number of bytes in record
   */
  void SetBinary(const char* data, std::size_t len);
    

  /**
   * Clear End type (resolve empty section)
   *
   */
  void ClrEnd(void);

  /**
   * Get integer value of a numeric token
   *
   * @return
   *   Token's integer value
   */
  Int IntegerValue(void) const;

  /**
   * Get float value of a numeric token
   *
   * @return
   *   Token float value
   */
  Float FloatValue(void) const;

  /**
   * Get string value of a name token
   *
   * @return
   *   Token's string value
   */
  const std::string& StringValue(void) const;

  /**
   * Get option value of a name token
   *
   * @return
   *   Token's option value
   */
  const std::string& OptionValue(void) const;
  
  /**
   * Get token Type
   *
   * This method is for backward compatibility only. It returns
   * a token type with only oe bit set to indicate the
   * type as in libfaudes up to  version 2.17. To test
   * for possible token interpretations, use the 2.17 interface
   * IsInteger(), IsString() etc.
   * @return
   *   Type of token
   */
  TokenType Type(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsNone(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsInteger(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsInteger16(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsBoolean(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsFloat(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsOption(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsString(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsBinary(void) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsBegin(void) const;

  /**
   * Test token Type
   *
   * @param tag
   *   Section tag to test for
   * @return
   *   True on match
   */
  bool IsBegin(const std::string& tag) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsEnd(void) const;

  /**
   * Test token Type
   *
   * @param tag
   *   Section tag to test for
   * @return
   *   True on match
   */
  bool IsEnd(const std::string& tag) const;

  /**
   * Test token Type
   *
   * @return
   *   True on match
   */
  bool IsEmpty(void) const;

  /**
   * Clear all attributes.
   *
   */
  void ClearAttributes();

  /**
   * Clear attribute.
   *
   * @param name
   *   Attribute name
   */
  void ClrAttribute(const std::string& name);

  /**
   * Insert named attribute, no type.
   * Note: only begin tags can have attributes.
   *
   * @param name
   *   Attribute name
   * @param value
   *   Attribute value 
   */
  void InsAttribute(const std::string& name, const std::string& value);

  /**
   * Insert named attribute with string value.
   * Note: only begin tags can have attributes.
   *
   * @param name
   *   Attribute name
   * @param value
   *   Attribute value 
   */
  void InsAttributeString(const std::string& name, const std::string& value);

  /**
   * Insert named attribute with integer value.
   * Note: only begin tags can have attributes.
   *
   * @param name
   *   Attribute name
   * @param value
   *   Attribute value 
   */
  void InsAttributeInteger(const std::string& name, Int value);

  /**
   * Insert named attribute with integer value.
   * Note: only begin tags can have attributes.
   *
   * @param name
   *   Attribute name
   * @param value
   *   Attribute value 
   */
  void InsAttributeInteger16(const std::string& name, Int value);

  /**
   * Insert named attribute with boolean value.
   * Note: only begin tags can have attributes.
   *
   * @param name
   *   Attribute name
   * @param value
   *   Attribute value 
   */
  void InsAttributeBoolean(const std::string& name, Int value);

  /**
   * Insert named attribute with integer value.
   * Note: only begin tags can have attributes.
   *
   * @param name
   *   Attribute name
   * @param value
   *   Attribute value 
   */
  void InsAttributeFloat(const std::string& name, Float value);

  /**
   * Test attibute existence.
   *
   * @param name
   *   Attribute name
   * @return
   *   True is attribute exists and matches type.
   */
  bool ExistsAttributeString(const std::string& name);

  /**
   * Test attibute existence.
   *
   * @param name
   *   Attribute name
   * @return
   *   True is attribute exists and matches type.
   */
  bool ExistsAttributeInteger(const std::string& name);

  /**
   * Test attibute existence.
   *
   * @param name
   *   Attribute name
   * @return
   *   True is attribute exists and matches type.
   */
  bool ExistsAttributeFloat(const std::string& name);

  /**
   * Access attribute value
   *
   * @param name
   *   Attribute name
   * @return
   *   String value of specified attribute
   */
  const std::string& AttributeStringValue(const std::string& name);

  /**
   * Access attribute value
   *
   * @param name
   *   Attribute name
   * @return
   *   Integer value of specified attribute (return 0 if it does not exist)
   */
  Int AttributeIntegerValue(const std::string& name);


  /**
   * Access attribute value
   *
   * @param name
   *   Attribute name
   * @return
   *   String value of specified attribute
   */
   Float AttributeFloatValue(const std::string& name);


  /**
   * Read Token from input stream
   * 
   * @param pStream
   *   Pointer to std::ifstream
   * @exception Exception
   *   - ios exceptions (eg file io error)
   * @return
   *   line count
   */
  int Read(std::istream* pStream);

  /**
   * Write Token to output stream
   *
   * @param pStream 
   *   Pointer to ostream
   * @exception Exception
   *   - ios exceptions (eg file io error,)
   */
  void Write(std::ostream* pStream) const; 


  /** Write specified binary data as base64 string to output stream
   *
   * @param pStream
   *   Reference to std::ostream
   * @param len
   *   Number of bytes to write
   * @param pData
   *   Data to write
   */
  static void WriteBinary(std::ostream* pStream, const char* pData, std::size_t len);

  /** Write a std::string value to an output stream.
   *
   * This method writes a string verbatim, i.e. incl all control characters.
   * It is enclosed by a marker which defaults to "__VERBATIM__". If the string
   * contains the marker, a variation is used.
   *
   * @param pStream
   *   Reference to std::ostream
   * @param rString
   *   String to write
   */
  static void WriteVerbatim(std::ostream* pStream, const std::string& rString);

  /** Write a std::string value to an output stream.
   *
   * This method replace critical characters by their XML entities and
   * streams the resulting string. No whitespace etc added.
   *
   * @param pStream
   *   Reference to std::ostream
   * @param outstr
   *   String to stream
   * @return
   *   Number of characters written.
   */
  static int WriteEscapedString(std::ostream* pStream, const std::string& outstr);


  /** Read a std::string value from an input file stream.
   *
   * Read an XML escaped string until and excl. the specified stop character.
   *
   * @param pStream
   *   Reference to std::istream
   * @param stop
   *   Stop character
   * @param rString
   *   Reference to result.
   * 
   * @return 
   *   Line count or -1 for error
   */
  static int ReadEscapedString(std::istream* pStream, char stop, std::string& rString);

  /** Read chracter data from an input file stream.
   *
   * Reads the stream untion the next "<" character.
   * The plain character data is returned, no enteties substituted etc. 
   *
   * @param pStream
   *   Reference to std::istream
   * @param rString
   *   Reference to result.
   * 
   * @return 
   *   Line count or -1 for error
   */
  static int ReadCharacterData(std::istream* pStream, std::string& rString);

  /**
   * Pretty print string representation
   *
   * Convenience functio for inspection/debugging
   *
   * @return
   *   Printable string representation
   *
   */
  std::string Str(void) const;


private:

  /** Token type */
  int mType; 
		
  /** Token std::string value (for any token type) */
  std::string mStringValue;

  /** Token std::string value (if token is of type Option) */
  std::string mOptionValue;

  /** Token integer value (if Token is of type Integer or Integer16) */
  Int mIntegerValue;

  /** Token float value (if Token is of type Float or Integer) */
  Float mFloatValue;

  /** When read from stream, record preceeding space */
  bool mPreceedingSpace;

  /** When read from stream, record preceeding space */
  bool mPreceedingNewline;

  /** Attribute value */
  class AttributeValue {
  public:
    AttributeValue(void) {mType=None;}
    std::string mStringValue;
    Int mIntegerValue;
    Float mFloatValue;
    int mType;
    unsigned int mSort; 
  };

  /** Attribute value map */
  std::map<std::string, AttributeValue> mAttributes;

  /** Attribute sort index (for nice output only) */
  int mAttributeCount;

  /** Convenience typedef */
  typedef std::map<std::string, AttributeValue>::iterator aiterator;
  typedef std::map<std::string, AttributeValue>::const_iterator caiterator;

  /** Interpret attribute value from string */
  void InterpretAttribute(aiterator ait);

  /** Interpret string a s number */
  bool InterpretNumber(const std::string& numstr, int& type, Int& ival, Float& fval); 

  /** Interpret string a s number */
  bool InterpretNumber(void); 

  /** Write a std::string value to an output stream.
   *
   * This method writes the string enclosed by a the specified
   * delimiter, typically '"' or ' '. Relevant XML entities are 
   * replaced by references, e.g. &amp;lt; &amp;&amp; etc. A
   * single white space is added as a sepqrqtor.
   *
   * @param pStream
   *   Reference to std::ostream
   * @param delim
   *   Delimiter
   */
  void WriteString(std::ostream* pStream, const std::string& delim) const;

  /** Write my binary data as base64 string to output stream
   *
   * @param pStream
   *   Reference to std::ostream
   */
  void WriteBinary(std::ostream* pStream) const;

  /** Read a std::string value from an input file stream.
   *
   * This method assumes that the string was written in the format
   * of WriteString, i.e. enclosed by single stop characters. However,
   * for practical reasons, it is assumed that the first stop character has
   * been allready read .
   *
   * @param pStream
   *   Reference to std::istream
   * @param stop
   *   Stop character
   * 
   * @return 
   *   Line count or -1 for error
   */
  int ReadString(std::istream* pStream, char stop);

  /** Read and interpret attribute definitions of begin tags from an input file stream.
   *
   *
   * @param pStream
   *   Reference to std::istream
   * 
   * @return 
   *   Line count or -1 for error
   */
  int ReadAttributes(std::istream* pStream);

  /** Read and interpret markup an input file stream.
   *
   * This method will identify begin and end tags. Any other XML
   * markup is meant to be gracefully ignored be ignored.
   *
   * @param pStream
   *   Reference to std::istream
   * 
   * @return 
   *   Line count or -1 for error
   */
  int ReadMarkup(std::istream* pStream);

  /** Read a std::string value from an input file stream.
   *
   * This method assumes that the string was written in the format
   * of WriteVerbatim, i.e. enclosed by a start and stop markers
   * "__VERBATIM__" or variations thereof. However, for practical reasons, 
   * it is assumed that the first character "_" has been read allready.
   * Note: verbatim sections are in general *NOT* XML compliant.
   *
   * @param pStream
   *   Reference to std::istream
   * 
   * @return 
   *   Line count or -1 for error
   */
  int ReadVerbatim(std::istream* pStream);

  /**
   * Read a base64 binary string from an input file stream
   * 
   * @param pStream
   *   Reference to std::istream
   * 
   * @return 
   *   Line count or -1 for error
   */
  int ReadBinary(std::istream* pStream); 

  /**
   * Read (ignore) spaces and comments in an input file stream
   *
   * @param pStream
   *   Reference to std::istream
   *
   * @return
   *   Number of lines read
   */   
  int  ReadSpace(std::istream* pStream);
};

} // namespace faudes

#define FAUDES_TOKEN_H
#endif


