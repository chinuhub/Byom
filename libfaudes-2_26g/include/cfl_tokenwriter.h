/** @file cfl_tokenwriter.h @brief Class TokenWriter */

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


#ifndef FAUDES_TOKENWRITER_H

#include "cfl_definitions.h"
#include "cfl_token.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>

namespace faudes {

/**
 * A TokenWriter writes sequential tokens to a file, a string or stdout. It is the counterpart
 * of the TokenReader. Since wrtiting data comparatively straight foreward, there is no explicit 
 * support of sections etc. It is left to the calling function to organise the output.
 * 
 * @ingroup TokenIO
 */

class TokenWriter {

 public:

  /**
   * Mode of operation: write to file, string or stdout
   */
  enum Mode {File, XmlFile, Stdout, String, Stream, XmlStream};

  /**
   * Console or String TokenWriter constructor
   *
   * Technical detail: Stdout mode uses the global console object
   * ConsoleOut::G()  declared in cfl_helper.h.
   *
   * @exception Exception
   *   - faudes::Exception ios error opening file (id 2)
   */
  TokenWriter(Mode mode);

  /** 
   * File TokenWriter constructor
   *
   * @param rFilename
   *   File to write
   * @param openmode
   *   std::ios::openmode
   * @exception Exception
   *   - faudes::Exception ios error opening file (id 2)
   */
  TokenWriter(const std::string& rFilename, 
	      std::ios::openmode openmode = std::ios::out|std::ios::trunc);

  /** 
   * Xml File TokenWriter constructor
   *
   * @param rFilename
   *   File to write
   * @param doctype
   *   String to indicate XML doctype.
   * @exception Exception
   *   - faudes::Exception ios error opening file (id 2)
   */
  TokenWriter(const std::string& rFilename, const std::string& doctype);

  /** 
   * Stream TokenWriter constructor
   *
   * @param rStream
   *   stream C++ stream to write to
   * @param doctype
   *   String to indicate XML doctype.
   * @exception Exception
   *   - faudes::Exception ios error opening file (id 2)
   */
  TokenWriter(std::ostream& rStream, const std::string& doctype="");

  /** Destructor. Calls close */
  ~TokenWriter(void);

  /**
   * Get the filename.
   * Return dummy values for console or string mode.
   * 
   * @return 
   *   Filename
   */
  std::string FileName(void) const;

  /**
   * Flush any buffers.
   * 
   */
  void Flush(void);

  /**
   * Test for file mode (incl. XmlFile)
   * 
   * @return 
   *   Mode
   */
  bool FileMode(void) const { return mMode==File || mMode==XmlFile ;};

  /**
   * Test for xml file mode
   * 
   * @return 
   *   Mode
   */
  bool XmlMode(void) const { return mMode==XmlFile ;};

  /**
   * Test for file mode.
   * 
   * @return 
   *   Mode
   */
  bool StdoutMode(void) const { return mMode==Stdout;};

  /** 
   * Retrieve output as string (if in String mode)
   *
   * @exception Exception
   *   - faudes::Exception not in string mode (id 2)
   */
  std::string Str(void);
	
  /** 
   * Access C++ stream
   *
   */
  std::ostream* Streamp(void);

  /**
   * Get number of columns in a line
   *
   * @return
   *   # of columns in a line
   */
  int Columns(void) const;

  /**
   * Set number of columns in a line 
   *
   * @param columns
   *   # of columns in a line
   */
  void Columns(int columns);

  /**
   * Write endl separator
   *
   * @exception Exception
   *   - faudes::Exception ios error writing file (id 2)
   */
  void Endl(void);

  /**
   * Turn endl separator on/off
   *
   */
  void Endl(bool on);

  /**
   * Write next token
   *
   * @param rToken
   *   Token to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void Write(const Token& rToken);

  /**
   * Write string.
   *
   * Writes a std string token, i.e. enclosed in double quotes
   * any quotes in the string will be escaped, controls ignored.
   *
   * @param rString
   *   String to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteString(const std::string& rString);

  /**
   * Write text.
   *
   * Writes the specified string. Relevant enteties are escaped. 
   *
   * @param rText
   *   String to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteText(const std::string& rText);

  /**
   * Write text section.
   *
   * Writes the specified string eclosed in begin/end tags.
   *
   * @param rLabel
   *   String to write
   * @param rText
   *   String to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteText(const std::string& rLabel, const std::string& rText);

  /**
   * Write text section.
   *
   * Writes the specified string eclosed in begin/end tags.
   * Use this version to have attributes in the begin tag.
   *
   * @param rBeginTag
   *   Begin tag.
   * @param rText
   *   String to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteText(const Token& rBeginTag, const std::string& rText);

  /**
   * Write character data
   *
   * Writes the specified string as it is. Thus, relevant enteties 
   * must be escaped befrand.
   *
   * @param rCharData
   *   String to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteCharacterData(const std::string& rCharData);

  /**
   * Write string. 
   *
   * Writes string enclosed in verbatim markes __VERBATIM__, incl 
   * controls.
   *
   * @param rString
   *   String to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   *   - faudes::Exception tag is not a begin tag  (id 2)
   */
  void WriteVerbatim(const std::string& rString);

  /**
   * Write non negative integer 
   *
   * @param index
   *   Integer to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteInteger(Idx index);

  /**
   * Write float
   *
   * @param val
   *   float to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteFloat(const double& val);

  /**
   * Write integer as hex
   *
   * @param val
   *   Integer to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteInteger16(long int val);

  /**
   * Write option (may not contain any "+")
   *
   * @param rOpt
   *   option to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteOption(const std::string& rOpt);

  /**
   * Write begin label
   * 
   * @param rLabel
   *   End label, e.g. "Alphabet"
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteBegin(const std::string& rLabel);

  /**
   * Write end label
   * 
   * @param rLabel
   *   End label, e.g. "Alphabet"
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteEnd(const std::string& rLabel);
	
  /**
   * Write empty section label
   * 
   * @param rLabel
   *   End label, e.g. "Alphabet"
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteEmpty(const std::string& rLabel);

  /**
   * Write comment in faudes format
   * 
   * @param rComment
   *   Comment to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteComment(const std::string& rComment);

  /**
   * Write comment in Xml format
   * 
   * @param rComment
   *   Comment to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteXmlComment(const std::string& rComment);

  /**
   * Write comment
   * 
   * @param len
   *   Number of bytes to write
   * @param pData
   *   Data to write
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  void WriteBinary(const char* pData, long int len);

  /** 
   * Operator for writing tokens
   *
   * @param rToken
   *   Token to write
   * @return
   *   Reference to this TokenWriter
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  TokenWriter& operator << (Token& rToken) {
    Write(rToken);
    return *this;
  }

  /** 
   * Operator for writing std::strings to a stream 
   *
   * @param rString
   *   String to write
   * @return
   *   Reference to this TokenWriter
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  TokenWriter& operator << (const std::string& rString) {
    WriteString(rString);
    return *this;
  }

  /** 
   * Operator for writing Idxs to a stream
   *
   * @param index
   *   Index to write
   * @return
   *   Reference to this TokenWriter
   * @exception Exception
   *   - faudes::Exception ios error wrtiting file (id 2)
   */
  TokenWriter& operator << (const Idx index) {
    WriteInteger(index);
    return *this;
  }


 private:
  /** Output mode */
  Mode mMode;

  /** ostream object pointer*/
  std::ostream* mpStream;

  /** Actual stream object, file output */
  std::ofstream mFStream;

  /** Actual stream object, string output */
  std::ostringstream mSStream;

  /** Actual stream object, stream output */
  std::ostream* pSStream;

  /** Outputbuffer */
  Token mOutBuffer;
  bool mHasOutBuffer;

  /** Filename */
  std::string mFileName;

  /** Number of columns */
  int mColumns;

  /** Column counter */
  int mColCount;

  /** Endl seperator on/off */
  bool mEndl;

  /** Xml doctype if in xml mode */
  std::string  mDocType;

  /** Flush internal buffer */
  void DoFlush(void);

};

} // namespace faudes

#define FAUDES_TOKENWRITER_H
#endif 

