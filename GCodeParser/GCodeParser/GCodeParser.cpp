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
 *
 */

#include "GCodeParser.h"
#include <stdlib.h>
#include <string.h>

 /// <summary>
 /// Initializizes class.
 /// </summary>
void GCodeParser::Initialize()
{
	lineCharCount = 0;
	line[lineCharCount] = '\0';
	comments = line;
	blockDelete = false;
	completeLineIsAvailableToParse = false;
}

/// <summary>
/// Class constructor.
/// </summary>
/// <remark>
/// The G Code language is based on the RS274/NGC language. The G Code 
/// language is based on lines of code. Each line (also called a 'block')
/// may include commands to do several different things. Lines of code
/// may be collected in a file to make a program.
/// </remark>
GCodeParser::GCodeParser()
{
	lastChar = '\0';

	Initialize();
}

/// <summary>
/// Adds a character to the line to be parsed.
/// </summary>
/// <param name="letter">The character to add.</param>
/// <returns>True if a complete line is available to parse.</returns>
/// <remarks>Adding a character after a CR/LF or LF have been added will reset the line buffer.</remarks>
bool GCodeParser::AddCharToLine(char c)
{
	// Look for end of line. CRLF or just LF.
	if (c == '\r' || c == '\n')
	{
		// If the last charater was not CR we are dealing with a Linux type file with lines ending in LF.
		// If the last character was CR we are dealing with a Windows type file with lines ending in CRFL
		// and have already processed the available line.
		if (lastChar != '\r')
		{
			line[lineCharCount] = '\0';
			completeLineIsAvailableToParse = true;
		}
		else
		{
			// Reset the line buffer and start a new line.
			Initialize();
		}
	}
	else
	{
		// Determine is a new line is being added.
		if (completeLineIsAvailableToParse)
			Initialize();

		// Add character to line.
		line[lineCharCount] = c;
		lineCharCount++;

		// Deal with buffer overflow by initializing. TODO: Need a better solution.  i.e. Throw error?
		if (lineCharCount > MAX_LINE_SIZE)
			Initialize();
	}

	lastChar = c;

	return completeLineIsAvailableToParse;
}

/// <summary>
/// Parses the line removing spaces, tabs and comments. Comments are shifted to the end of the line buffer.
/// </summary>
void GCodeParser::ParseLine()
{
	int lineLength = strlen(line);
	line[lineLength + 1] = '\0';

	int pointer = 0;
	bool openParentheseFound = false;
	bool semicolonFound = false;
	int correctCommentsPointerBy = 0;

	while (line[pointer] != '\0')
	{
		char c = line[pointer];

		// Look for end of comment.
		if (c == '(')
			openParentheseFound = true; // Open parenthese... start of comment.

		if (c == ';')
			semicolonFound = true; // Semicolon... start of comment to end of line.

		// If we are inside a comment, we need to move it to the end of the buffer in order to seperate it.
		if (openParentheseFound || semicolonFound)
		{
			// Shift line left.
			for (int i = pointer; i < lineLength; i++)
			{
				line[i] = line[i + 1];
			}
			line[lineLength] = c;
		}
		else
		{
			// Spaces and tabs are allowed anywhere on a line of code and do not change the meaning of 
			// the line, except inside comments. Remove spaces and tabs except in comments. 
			if (c == ' ' || c == '\t')
			{
				int removeCharacterPointer = pointer;

				while (line[removeCharacterPointer] != '\0')
				{
					line[removeCharacterPointer] = line[removeCharacterPointer + 1];

					removeCharacterPointer++;
				}

				correctCommentsPointerBy++;
			}
			else
				pointer++;
		}

		// Look for end of comment.
		if (c == ')')
		{
			openParentheseFound = false;

			// Is this the end of the comment? Scan forward for second closing parenthese, but no opening parenthese first.
			int scanAheadPointer = pointer;

			while (line[scanAheadPointer] != '\0')
			{
				if (line[scanAheadPointer] == '(')
					break;

				if (line[scanAheadPointer] == ')')
				{
					openParentheseFound = true;
					break;
				}

				scanAheadPointer++;
			}
		}
	}

	// Set pointer to comments.
	comments = line + strlen(line) + correctCommentsPointerBy + 1;

	// The optional block delete character the slash '/' when placed first on a line can be used
	// by some user interfaces to skip lines of code when needed.
	if (line[0] == '/')
		blockDelete = true;
}

/// <summary>
/// Looks for a word in the line.
/// </summary>
/// <param name="c">The letter of the word to look for in the line.</param>
/// <returns>A pointer to where the word starts.  Points to \0 if the word was not found.</returns>
int GCodeParser::FindWord(char letter)
{
	int pointer = 0;
	bool openParentheseFound = false;
	bool semicolonFound = false;

	while (line[pointer] != '\0')
	{
		// Look for the word.
		if (letter == line[pointer])
		{
			return pointer;
		}

		pointer++;
	}

	return pointer;
}

/// <summary>
/// Looks through the code block to determin if a word exist.
/// </summary>
/// <param name="letter">The letter of the GCode word.</param>
/// <returns>True if the word exist on the line.</returns>
bool GCodeParser::HasWord(char letter)
{
	if (IsWord(letter))
	{
		int pointer = FindWord(letter);

		if (line[pointer] == '\0')
		{
			return false;
		}
	}

	return true;
}

/// <summary>
/// Determine if the letter provided represents a valid GCode word.
/// </summary>
/// <param name="letter">The letter to be tested.</param>
/// <returns>True if the letter represents a valid word. </returns>
/// <remark>
/// Words may begin with any of the letters shown in the following
/// Table. The table includes N, @ and ^ for completeness, even though,
/// line numbers and polar coordinates are not words. Several letters 
/// (I, J, K, L, P, R) may have different meanings in different contexts.
/// Letters which refer to axis names are not valid on a machine which
/// does not have the corresponding axis.
/// 
/// A - A axis of machine.
/// B - B axis of machine.
/// C - C axis of machine.
/// D - Tool radius compensation number.
/// F - Feed rate.
/// G - General function(See table Modal Groups).
/// H - Tool length offset index.
/// I - X offset for arcsand G87 canned cycles.
/// J - Y offset for arcsand G87 canned cycles.
/// K - Z offset for arcsand G87 canned cycles. Spindle - Motion Ratio for G33 synchronized movements.
/// L - generic parameter word for G10, M66and others.
/// M - Miscellaneous function(See table Modal Groups).
/// N - Line number. Line numbers are not considered words.
/// P - Dwell time in canned cyclesand with G4. Key used with G10.
/// Q - Feed increment in G73, G83 canned cycles.
/// R - Arc radius or canned cycle plane.
/// S - Spindle speed.
/// T - Tool selection.
/// U - U axis of machine.
/// V - V axis of machine.
/// W - W axis of machine.
/// X - X axis of machine.
/// Y - Y axis of machine.
/// Z - Z axis of machine
/// @ - Polar coordinate for the distance. Polar coordinates are not considered words.
/// ^ - Polar coordinate for the angle. Polar coordinates are not considered words.
/// /// </remark>
bool GCodeParser::IsWord(char letter)
{
	char wordLetter[] = { 'A', 'B', 'C', 'D', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0' };

	int pointer = 0;
	while (wordLetter[pointer] != '\0')
	{
		if (letter == wordLetter[pointer])
		{
			return true;
		}

		pointer++;
	}

	return false;
}

/// <summary>
/// Gets the value following the word.
/// </summary>
/// <param name="letter">The letter of the word to look for in the line.</param>
/// <returns>The value following the letter for the word.</returns>
/// <remarks>
/// Currently the parser is not sophisticated enough to deal with parameters, 
/// Boolean operators, expressions, binary operators, functions and repeated items.
/// </remarks>
double GCodeParser::GetWordValue(char letter)
{
	int pointer = FindWord(letter);

	if (line[pointer] != '\0')
		return (double)strtod(&line[pointer + 1], NULL);

	return 0.0;
}
