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

class GCodeParser
{
private:
	char lastChar;
	bool completeLineIsAvailableToInterpret;

	int lineCharCount;
	int codeBlockCharCount;
	int commentCharCount;

	void Initialize();
	int FindWordInCodeBlock(char letter);
public:
	char line[MAX_LINE_SIZE + 1];
	char codeBlock[MAX_LINE_SIZE + 1];
	char comment[MAX_LINE_SIZE + 1];

	GCodeParser();
	bool AddCharToLine(char c);

	bool HasWord(char letter);
	bool IsWord(char letter);

	double GetWordValue(char letter);
};

#endif