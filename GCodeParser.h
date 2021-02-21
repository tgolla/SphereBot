/*
 * Copyright 2021 by Terence Golla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef GCodeParser_h
#define GCodeParser_h

const int MAX_LINE_SIZE = 256; // Maximun GCode line size.

/// <summary>
/// The GCode parser is designed to parse GCode in order that it can then be processed.
/// The parser is meant to be light weight using only one buffer to first collect a
/// line (also called a 'block') from serial or file input and then parse that line
/// into a code block and comments. The parser was originally designed for use with 
/// code for the SphereBot, an EggBot clone.
/// 
/// Currently the parser is not sophisticated enough to deal with parameters, 
/// Boolean operators, expressions, binary operators, functions and repeated items.
/// However, this should not be an obstacle when building 2D/3D plotters, CNC, and 
/// projects with an Arduino controller.
/// 
/// The following are just a few sources of information on GCode.
/// https://www.autodesk.com/products/fusion-360/blog/cnc-programming-fundamentals-g-code/
/// http://www.machinekit.io/docs/gcode/overview/
/// https://www.reprap.org/wiki/G-code
/// https://howtomechatronics.com/tutorials/g-code-explained-list-of-most-important-g-code-commands/
/// 
/// </summary>
class GCodeParser
{
private:
	int lineCharCount;

	char lastChar;
	bool completeLineIsAvailableToParse;

	void Initialize();
public:
	char line[MAX_LINE_SIZE + 2];
	char* comments;
	char* lastComment;
	bool blockDelete;

	GCodeParser();
	bool AddCharToLine(char c);
	void ParseLine();
	void RemoveCommentSeparators();

	int FindWord(char letter);
	bool HasWord(char letter);
	bool IsWord(char letter);

	double GetWordValue(char letter);
};

#endif