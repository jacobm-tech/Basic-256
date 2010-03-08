/**
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 2 of the License, or
 **  (at your option) any later version.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 **/

#include <QTextDocument>

#include "EditSyntaxHighlighter.h" 
 
EditSyntaxHighlighter::EditSyntaxHighlighter(QTextDocument *parent)
  : 	QSyntaxHighlighter(parent)
{
  initKeywords();
  initColors();
  initQuotes();
  initFunctions();	
  initComments();
}

void EditSyntaxHighlighter::highlightBlock(const QString &text)
{
  HighlightRule rule;

  VecHighlightRules::iterator sIt = m_standardRules.begin();
  VecHighlightRules::iterator sItEnd = m_standardRules.end();
  while (sIt != sItEnd) 
    {
      rule = (*sIt);
      QRegExp expression(rule.pattern);
      int index = text.indexOf(expression);
      while (index >= 0) {
	int length = expression.matchedLength();
	setFormat(index, length, rule.format);
	index = text.indexOf(expression, index + length);
      }
      ++sIt;
    }
	
  // Now do the functions, checking for calls to any matches.
  VecHighlightRules::iterator fIt = m_functionRules.begin();
  VecHighlightRules::iterator fItEnd = m_functionRules.end();
  while (fIt != fItEnd)
    {
      rule = (*fIt);
      QRegExp expression(rule.pattern);
      int index = text.indexOf(expression);
      while (index >= 0) {
	int length = expression.matchedLength();
	setFormat(index, length, rule.format);
	index = text.indexOf(expression, index + length);
      }		
      ++fIt;
    }	
}

void EditSyntaxHighlighter::initKeywords()
{
  HighlightRule rule;
  
  m_keywordFmt.setForeground(Qt::darkBlue);
  QStringList keywordPatterns;
  
  keywordPatterns
		  << "\\b[Aa][Bb][Ss]\\b"						// abs
		  << "\\b[Aa][Nn][Dd]\\b"						// and
		  << "\\b[Aa][Ss][Cc]\\b"						// asc
		  << "\\b[Cc][Ee][Ii][Ll]\\b"						// ceil
		  << "\\b[Cc][Hh][Rr]\\b"						// chr
		  << "\\b[Cc][Ii][Rr][Cc][Ll][Ee]\\b"					// circle
		  << "\\b[Cc][Ll][Ee][Aa][Rr]\\b"					// clear
		  << "\\b[Cc][Ll][Ii][Cc][Kk][Bb]\\b"					// clickb
		  << "\\b[Cc][Ll][Ii][Cc][Kk][Cc][Ll][Ee][Aa][Rr]\\b"			// clickclear
		  << "\\b[Cc][Ll][Ii][Cc][Kk][Xx]\\b"					// clickx
		  << "\\b[Cc][Ll][Ii][Cc][Kk][Yy]\\b"					// clicky
		  << "\\b[Cc][Ll][Oo][Ss][Ee]\\b"					// close
		  << "\\b[Cc][Ll][Ss]\\b"						// cls
		  << "\\b[Cc][Ll][Gg]\\b"						// clg
		  << "\\b[Cc][Oo][Ll][Oo][Rr]\\b"					// color
		  << "\\b[Cc][Oo][Ll][Oo][Uu][Rr]\\b"					// colour
		  << "\\b[Cc][Oo][Ss]\\b"						// cos
		  << "\\b[Dd][Aa][Yy]\\b"						// day
		  << "\\b[Dd][Ii][Mm]\\b"						// dim
		  << "\\b[Dd][Oo]\\b"							// do
		  << "\\b[Ee][Ll][Ss][Ee]\\b"						// else
		  << "\\b[Ee][Oo][Ff]\\b"						// eof
		  << "\\b[Ee][Nn][Dd]\\b"						// end
		  << "\\b[Ee][Nn][Dd][Ii][Ff]\\b"					// endif
		  << "\\b[Ee][Nn][Dd][Ww][Hh][Ii][Ll][Ee]\\b"				// endwhile
		  << "\\b[Ee][Xx][Ii][Ss][Tt][Ss]\\b"					// exists
		  << "\\b[Ff][Aa][Ss][Tt][Gg][Rr][Aa][Pp][Hh][Ii][Cc][Ss]\\b"		// fastgraphics
		  << "\\b[Ff][Ll][Oo][Oo][Rr]\\b"					// floor
		  << "\\b[Ff][Oo][Rr]\\b"						// for
		  << "\\b[Gg][Ee][Tt][Cc][Oo][Ll][Oo][Rr]\\b"					// getcolor
		  << "\\b[Gg][Oo][Ss][Uu][Bb]\\b"					// gosub
		  << "\\b[Gg][Oo][Tt][Oo]\\b"						// goto
		  << "\\b[Gg][Rr][Aa][Pp][Hh][Hh][Ee][Ii][Gg][Hh][Tt]\\b"   		// graphheignt
		  << "\\b[Gg][Rr][Aa][Pp][Hh][Ss][Ii][Zz][Ee]\\b"   			// graphsize
		  << "\\b[Gg][Rr][Aa][Pp][Hh][Ww][Ii][Dd][Tt][Hh]\\b"   		// graphwidth
		  << "\\b[Hh][Oo][Uu][Rr]\\b"						// hour
		  << "\\b[Ii][Ff]\\b"							// if
		  << "\\b[Ii][Nn][Pp][Uu][Tt]\\b"					// input
		  << "\\b[Ii][Nn][Ss][Tt][Rr]\\b"					// instr
		  << "\\b[Kk][Ee][Yy]\\b"						// key
		  << "\\b[Ll][Ee][Ff][Tt]\\b"						// left
		  << "\\b[Ll][Ii][Nn][Ee]\\b"						// line
		  << "\\b[Ll][Ee][Nn][Gg][Tt][Hh]\\b"					// length
		  << "\\b[Ll][Oo][Ww][Ee][Rr]\\b"					// lower
		  << "\\b[Mm][Ii][Dd]\\b"						// mid
		  << "\\b[Mm][Ii][Nn][Uu][Tt][Ee]\\b"					// minute
		  << "\\b[Mm][Oo][Nn][Tt][Hh]\\b"					// month
		  << "\\b[Mm][Oo][Uu][Ss][Ee][Bb]\\b"					// mouseb
		  << "\\b[Mm][Oo][Uu][Ss][Ee][Xx]\\b"					// mousex
		  << "\\b[Mm][Oo][Uu][Ss][Ee][Yy]\\b"					// mousey
		  << "\\b[Nn][Ee][Xx][Tt]\\b"						// next
		  << "\\b[Nn][Oo][Tt]\\b"						// not
		  << "\\b[Oo][Pp][Ee][Nn]\\b"						// open
		  << "\\b[Oo][Rr]\\b"							// or
		  << "\\b[Pp][Aa][Uu][Ss][Ee]\\b"					// pause
		  << "\\b[Pp][Ii]\\b"							// pi
		  << "\\b[Pp][Ii][Xx][Ee][Ll]\\b"							// pixel
		  << "\\b[Pp][Ll][Oo][Tt]\\b"						// plot
		  << "\\b[Pp][Oo][Ll][Yy]\\b"						// poly
		  << "\\b[Pp][Rr][Ii][Nn][Tt]\\b"					// print
		  << "\\b[Rr][Aa][Nn][Dd]\\b"						// rand
		  << "\\b[Rr][Ee][Aa][Dd]\\b"						// read
		  << "\\b[Rr][Ee][Aa][Dd][Ll][Ii][Nn][Ee]\\b"				// readline
		  << "\\b[Rr][Ee][Cc][Tt]\\b"						// rect
		  << "\\b[Rr][Ee][Ff][Rr][Ee][Ss][Hh]\\b"				// refresh
		  << "\\b[Rr][Ee][Ss][Ee][Tt]\\b"					// reset
		  << "\\b[Rr][Ee][Tt][Uu][Rr][Nn]\\b"	        			// return
		  << "\\b[Rr][Gg][Bb]\\b"					// rgb
		  << "\\b[Rr][Ii][Gg][Hh][Tt]\\b"					// right
		  << "\\b[Ss][Aa][Yy]\\b"						// say
		  << "\\b[Ss][Ee][Cc][Oo][Nn][Dd]\\b"					// second
		  << "\\b[Ss][Ee][Ee][Kk]\\b"						// seek
		  << "\\b[Ss][Ii][Nn]\\b"						// sin
		  << "\\b[Ss][Ii][Zz][Ee]\\b"						// size
		  << "\\b[Ss][Tt][Aa][Mm][Pp]\\b"					// stamp
		  << "\\b[Ss][Tt][Ee][Pp]\\b"						// step
		  << "\\b[Ss][Yy][Ss][Tt][Ee][Mm]\\b"					// system
		  << "\\b[Tt][Aa][Nn]\\b"						// tan
		  << "\\b[Tt][Hh][Ee][Nn]\\b"						// then
		  << "\\b[Tt][Oo]\\b"							// to
		  << "\\b[Ff][Ll][Oo][Aa][Tt]\\b"					// tofloat
		  << "\\b[Ii][Nn][Tt]\\b"						// toint
		  << "\\b[Ss][Tt][Rr][Ii][Nn][Gg]\\b"					// tostring
		  << "\\b[Uu][Nn][Tt][Ii][Ll]\\b"					// until
		  << "\\b[Uu][Pp][Pp][Ee][Rr]\\b"					// upper
		  << "\\b[Vv][Oo][Ll][Uu][Mm][Ee]\\b"					// volume
		  << "\\b[Ww][Aa][Vv][Pp][Ll][Aa][Yy]\\b"				// wavplay
		  << "\\b[Ww][Aa][Vv][Ss][Tt][Oo][Pp]\\b"				// wavstop
		  << "\\b[Ww][Hh][Ii][Ll][Ee]\\b"					// while
		  << "\\b[Ww][Rr][Ii][Tt][Ee]\\b"					// write
		  << "\\b[Ww][Rr][Ii][Tt][Ee][Ll][Ii][Nn][Ee]\\b"			// writeline
		  << "\\b[Xx][Oo][Rr]\\b"						// xor
		  << "\\b[Yy][Ee][Aa][Rr]\\b"						// year
		;
  for (QStringList::iterator it = keywordPatterns.begin(); it != keywordPatterns.end(); ++it)
    {
      rule.pattern = QRegExp(*it);
      rule.format  = m_keywordFmt;
      m_standardRules.append(rule);
    }
	
}

void EditSyntaxHighlighter::initColors()
{
  HighlightRule rule;

  m_colorFmt.setForeground(Qt::darkCyan);
  QStringList colorPatterns;
  
  colorPatterns << "\\b[Bb][Ll][Aa][Cc][Kk]\\b"			// black
		<< "\\b[Ww][Hh][Ii][Tt][Ee]\\b"			// white
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Rr][Ee][Dd]\\b"					// red and darkred
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Gg][Rr][Ee][Ee][Nn]\\b"			// green and darkgreen
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Bb][Ll][Uu][Ee]\\b"				// blue and darkblue
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Cc][Yy][Aa][Nn]\\b"				// cyan and darkcyan
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Pp][Uu][Rr][Pp][Ll][Ee]\\b"		// purple and darkpurple
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Yy][Ee][Ll][Ll][Oo][Ww]\\b"		// yellow and darkyellow
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Oo][Rr][Aa][Nn][Gg][Ee]\\b"		// orange and darkorange
		<< "\\b([Dd][Aa][Rr][Kk]){0,1}[Gg][Rr][AaEe][Yy]\\b";			// gray and darkgrey
	
  for (QStringList::iterator it = colorPatterns.begin(); it != colorPatterns.end(); ++it )
    {
      rule.pattern = QRegExp(*it);
      rule.format  = m_colorFmt;
      m_standardRules.append(rule);
    }
	
}

void EditSyntaxHighlighter::initQuotes()
{
  HighlightRule rule;

  m_quoteFmt.setForeground(Qt::darkRed);
  rule.pattern = QRegExp("\"[^\"]*\"");
  rule.format = m_quoteFmt;
  m_standardRules.append(rule);
}

void EditSyntaxHighlighter::initFunctions()
{
  HighlightRule rule;
	
  m_functionFmt.setForeground(Qt::blue);
  rule.pattern = QRegExp("\\b[A-Za-z0-9]+(?=\\:$)");
  rule.format = m_functionFmt;
  m_functionRules.append(rule);		
}

void EditSyntaxHighlighter::initComments()
{
  HighlightRule rule;
	
  m_commentFmt.setForeground(Qt::darkGreen);
  m_commentFmt.setFontItalic(true);
  rule.pattern = QRegExp("(([Rr][Ee][Mm][ ])|#)[^\n]*");
  rule.format = m_commentFmt;
  m_standardRules.append(rule);	
}
