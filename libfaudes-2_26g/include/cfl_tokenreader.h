/** @file cfl_tokenreader.h @brief Class TokenReader */

/* FAU Discrete Event Systems Library (libfaudes)

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



#ifndef FAUDES_TOKENREADER_H

#include <vector>

#include "cfl_definitions.h"
#include "cfl_exception.h"
#include "cfl_token.h"

namespace faudes {

/**
 * A TokenReader reads sequential tokens from a file or string. It can get or peek
 * the next token and it will track line numbers for informative diagnosis output.
 *
 * The class also implements nested sections. That is, you may search for a
 * (sub-)section within the current section and, hence, implement file formats that 
 * do not insist in a particular ordering of the sections (e.g. a Generator
 * consists of states, transitions and events, no matter in which order).
 *
 * Convenience functions are provided to read a token of a particular type and
 * throws faudes::Exception on token mismatch. You may catch the exception 
 * as follows:
 * \code 
 * try {
 *   some tokenreader operations
 * }
 * catch (faudes::Exception& ex) {
 *   cerr << "Error reading file (details: " << ex.What() << ")" << endl;
 * }
 * \endcode
 * Note that in addition to the documented execeptions all TokenReader functions 
 * do pass on faudes ios errors from the Token class.
 * 
 * @ingroup TokenIO
 */

class TokenReader {
public:

  /**
   * Mode of operation: read from file, stdin or string
   */
  enum Mode {File, Stdin, String};

  /**
   * TokenReader constructor
   *
   * @param mode
   *   select source: File, Stdin or String
   * @param rInString
   *   string to read from or filename
   *
   * @exception Exception
   *   - faudes::Exception ios error opening file (id 1)
   *   - faudes::Exception invalid mode (Stdin not implemented) (id 1)
   */
  TokenReader(Mode mode, const std::string& rInString="");

  /**
   * Creates a TokenReader for reading a file.
   *
   * @param rFilename
   *   File to read
   *
   * @exception Exception
   *   - faudes::Exception ios error opening file (id 1)
   */
  TokenReader(const std::string& rFilename);


  /**
   * Destruct
   */
  ~TokenReader(void);

  /** 
   * Access C++ stream
   *
   */
  std::istream* Streamp(void);

  /**
   * Get state of TokenReader stream
   *
   * @return
   *   std::stream.good()
   */
  bool Good(void) const;

  /**
   * Get the filename.
   * Returns dummy values for console or string mode.
   * 
   * @return 
   *   Filename
   */
  std::string FileName(void) const;

  /**
   * Get file mode.
   * 
   * @return 
   *   Mode
   */
  Mode SourceMode(void) const { return mMode;};

  /**
   * Peek next token. False indicates eof.
   *
   * @param token
   *   Reference to token
   * 
   * @exception Exception
   *   - faudes::Exception ios error reading file (id 1)
   *
   * @return
   *   True for a valid token, false for eof
   */
  bool Peek(Token& token);

  /**
   * Get next token. False indicates eof.
   *
   * @param token
   *   Reference to token
   * 
   * @exception Exception
   *   faudes exception ios (id 1)
   *
   * @return
   *   True for a valid token, false for eof
   */
  bool Get(Token& token);

  /**
   * Rewind stream (must be a seekable stream)
   *
   * @exception Exception
   *   - faudes::Exception ios error reading file (id 1)
   */
  void Rewind(void);

  /**
   * This function searches for the specified section on the current level, 
   * it skips any sections on levels below, and it will wrap to the begin 
   * of the current section. In the case of success, it returns true, else
   * false. In contrast to other token i/o methodes, this method will not throw
   * any execptions.
   *
   * @param rLabel
   *   Token label to specify section
   *
   * @return
   *   True if sectiob exists
   *
   */
  bool ExistsBegin(const std::string& rLabel);

  /**
   * Open a section by specified label. This function searches
   * for the section on this level, it skips any sections on levels
   * below this level, and it will wrap to the begin of the current 
   * section. In the case of success, the matching begin token is the 
   * last token read. After processing the section, a matching ReadEnd(label) 
   * must be called.
   *
   * @param rLabel
   *   Token label to specify section
   *
   * @exception Exception
   *   Section begin label not found (id 51)
   *
   */
  void ReadBegin(const std::string& rLabel);

  /**
   * Open a section by specified label. 
   *
   * This version ReadBegin(const std::string&) will return the actual
   * begin tag in its second argument.
   *
   * @param rLabel
   *   Token label to specify section
   * @param rToken
   *   Begin tag as found in token stream.
   *
   * @exception Exception
   *   Section begin label not found (id 51)
   *
   */
  void ReadBegin(const std::string& rLabel, Token& rToken);

  /**
   * Close the current section by matching the previous ReadBegin(). 
   * Reads all tokens up to and including end of current section.
   *
   * @param rLabel
   *   Token label to specify section
   *
   * @exception Exception
   *   Section end label not found (id 51)
   *
   */
  void ReadEnd(const std::string& rLabel);

  /**
   * Find specified begin label.
   * This function searches
   * for the section on this level and any descending level. 
   * It does not read the specified section tag, but stops just
   * one token before.
   *
   * Technical note:
   * Former versions of libFAUDES also read the actual begin token and
   * required a matching call of SeekEnd(). As of version 2.18a, this is not
   * supported anymore. The previous behaviour was rarely needed and can be
   * mimiqued by an ordinary ReadEnd() with a subsequent Recover(level).
   *
   * @param rLabel
   *   Label to specify section
   *
   * @exception Exception
   *   Section begin not found (id 51)
   *
   */
  void SeekBegin(const std::string& rLabel);

  /**
   * Find specified begin label.
   *
   * This version SeekBegin(const std::string&) will return the actual
   * begin tag in its second argument.
   *
   * @param rLabel
   *   Token label to specify section
   * @param rToken
   *   Begin tag as found in token stream.
   *
   * @exception Exception
   *   Section begin label not found (id 51)
   *
   */
  void SeekBegin(const std::string& rLabel, Token& rToken);

  /**
   * Peek a token and check whether it ends the specified section.
   * This function is meant for scanning a section with a while- construct.
   * \code
   * SeekBegin("MySec"); 
   * while(!Eos("MySec")) { 
   *   ... 
   * }; 
   * SeekEnd("MySec");
   * \endcode
   *
   * @param rLabel
   *   Token label to specify section
   *
   * @exception Exception
   *   Unexpected eof (id 51)
   *
   * @return
   *   True at end of section
   */
  bool Eos(const std::string& rLabel);

  /**
   * Read integer  token
   *		  
   * @exception Exception
   *   Token mismatch (id 50)
   *
   * @return
   *   value of index token
   */
  long int ReadInteger(void);

  /**
   * Read float token
   *		  
   * @exception Exception
   *   Token mismatch (id 50)
   *
   * @return
   *   value of index token
   */
  double ReadFloat(void);

  /**
   * Read string token
   *		  
   * @exception Exception
   *   Token mismatch (id 50)
   *
   * @return
   *   value of name token
   */
  const std::string& ReadString(void);

  /**
   * Read option token
   *		  
   * @exception Exception
   *   Token mismatch (id 50)
   *
   * @return
   *   value of name token
   */
  const std::string& ReadOption(void);

  /**
   * Read binary token. You can access the binary array
   * via StringValue();
   *		  
   * @return
   *   Binary data as std::string
   * @exception Exception
   *   Token mismatch (id 50)
   *
   */
  const std::string& ReadBinary(void);


  /**
   * Read plain text
   *		  
   * Read all text until and excluding the next markup.
   * This method interprets known enteties and swallows
   * beginning and ending white space. Effetively, ot is used
   * to read ASCII compatible text.
   *
   * @exception Exception
   *   Stream mismatch (id 50)
   *
   * @return
   *   Text read.
   */
  const std::string& ReadText(void);


  /**
   * Read plain text
   *		  
   * Read all text until and excluding the next markup.
   * This method does no interpretation whatever. It can be used
   * to implement carbon copy of text sections.
   *
   * @exception Exception
   *   Stream mismatch (id 50)
   *
   * @return
   *   Text read.
   */
  const std::string& ReadCharacterData(void);

  /**
   * Read XML section
   *		  
   * Reads the current section, including all character data and markup, 
   * until and excluding the matching end tag. This method does no interpretation whatever.
   * the result is a string that represents
   * the respective section in XML format.
   *
   * @exception Exception
   *   Stream mismatch (id 50)
   * @return
   *   Text read.
   */
  const std::string& ReadSection(void);


  /** Operator for get */
  bool operator >> (Token& token) {
    return Get(token);
  }

  /**
   * Return number of lines read
   * 
   * @return
   *   Number of lines read
   */ 
  int Line(void) const;

  /**
   * Return current level of section nesting
   * 
   * @return
   *   Number of lines read
   */ 
  int Level(void) const { return mLevel;};

  /**
   * Recover to specified section nesting
   *
   * The current implementation will mess up the Seek-stack. Thus,
   * Recover() only works when no Seekbegin() was used.
   * 
   * @return
   *   True on success
   */ 
  bool Recover(int level);

  /**
   * Return "filename:line"
   */
  std::string FileLine(void) const;

 private:

  /** input mode */
  Mode mMode;

  /** istream object pointer */
  std::istream* mpStream;

  /** actual stream object, file input  */
  std::ifstream mFStream;

  /** actual stream object on heap, string input  */
  std::istringstream* mpSStream;

  /** Filename */
  std::string mFileName;

  /** Line counter */
  int mLineCount;

  /** file position */
  long int mFilePos;

  /** Level (of nested sections)*/
  int mLevel;

  /** label of sections */
  std::vector<std::string> mLevelLabel;

  /** file positions of sections */
  std::vector<long int> mLevelPos;

  /** file line of sections */
  std::vector<long int> mLevelLine;

  /** level of recent seek */
  std::vector<int> mSeekLevel;

  /** recent string buffer */
  std::string mLastString;

  /** peek buffer */
  Token mPeekToken;
};

} // namespace faudes

#define FAUDES_TOKENREADER_H
#endif 
