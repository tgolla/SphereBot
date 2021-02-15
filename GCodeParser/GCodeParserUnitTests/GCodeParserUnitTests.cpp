#include "pch.h"
#include "CppUnitTest.h"
#include "../GCodeParser/GCodeParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GCodeParserUnitTests
{
	TEST_CLASS(GCodeParserUnitTests)
	{
	public:
		int AddLine(GCodeParser *gcode, int startAt)
		{
			// G01 (Comment Here) Z0.0
			// M300 S125 ;Comment to end of line.
			// G01 X3.2 Y1.5 Z5.0
			char lines[] = { 'G', '0', '1' , ' ', '(', 'C', 'o', 'm', 'm', 'e', 'n', 't', ' ',
				'H', 'e', 'r', 'e', ')',  'Z' , '0' , '.' , '0' , '\r', '\n', 'M', '3', '0', '0', 
				' ', 'S', '1', '2', '5', ' ', ';', 'C', 'o', 'm', 'm', 'e', 't', ' ', 't', 'o', 
				' ', 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'l', 'i', 'n', 'e', '.', '\n', 
				'G', '0','1',' ','X','3','.','2',' ','Y','1','.','5',' ', '(', 'C', 'o', 'm', 'm',
				'e', 'n', 't', ')', ' ', 'H', 'e', 'r', 'e', ')','Z','5','.','0', '\0'
			};

			bool completeLineIsAvailableToInterpret;
			while (lines[startAt] != '\0')
			{
				completeLineIsAvailableToInterpret = gcode->AddCharToLine(lines[startAt]);

				startAt++;

				if (completeLineIsAvailableToInterpret)
					return startAt;
			}

			// Force end of line.  Similar to end of file with no ending line characters.
			gcode->AddCharToLine('\n');

			return startAt;
		}


		TEST_METHOD(AddCharToLine_AddLines_ConfirmLines)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			char resultLine[] = { 'G', '0', '1' , ' ', '(', 'C', 'o', 'm', 'm', 'e', 'n', 't', ' ',
				'H', 'e', 'r', 'e', ')',  'Z' , '0' , '.' , '0' };

			int pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.line[pointer], resultLine[pointer]);

				pointer++;
			}

			startAt = AddLine(&GCode, startAt);

			char resultLine2[] = { 'M', '3', '0', '0', 	' ', 'S', '1', '2', '5', ' ', ';', 'C', 'o',
				'm', 'm', 'e', 't', ' ', 't', 'o', ' ', 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'l', 'i',
				'n', 'e', '.' };

			pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.line[pointer], resultLine2[pointer]);

				pointer++;
			}

			AddLine(&GCode, startAt);

			char resultLine3[] = { 'G', '0','1',' ','X','3','.','2',' ','Y','1','.','5',' ', '(', 'C',
				'o', 'm', 'm', 'e', 'n', 't', ')', ' ', 'H', 'e', 'r', 'e', ')','Z','5','.','0' };

			pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.line[pointer], resultLine3[pointer]);

				pointer++;
			}
		}

		TEST_METHOD(AddCharToLine_AddLines_ConfirmComments)
		{
			GCodeParser GCode = GCodeParser();

			Assert::Fail();
		}

		TEST_METHOD(IsWord_ValidWord_ReturnsTrue)
		{
			GCodeParser GCode = GCodeParser();

			Assert::AreEqual(GCode.IsWord('G'), true);
		}

		TEST_METHOD(IsWord_InvalidWord_ReturnsFalse)
		{
			GCodeParser GCode = GCodeParser();

			Assert::AreEqual(GCode.IsWord('E'), false);
		}
		
		TEST_METHOD(HasWord_NoLine_ReturnsFalse)
		{
			GCodeParser GCode = GCodeParser();

			Assert::AreEqual(GCode.HasWord('G'), false);
		}

		TEST_METHOD(HasWord_WordExist_ReturnsTrue)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			Assert::AreEqual(GCode.HasWord('G'), true);
			Assert::AreEqual(GCode.HasWord('Z'), true);

			startAt = AddLine(&GCode, startAt);

			Assert::AreEqual(GCode.HasWord('M'), true);
			Assert::AreEqual(GCode.HasWord('S'), true);

			AddLine(&GCode, startAt);

			Assert::AreEqual(GCode.HasWord('Y'), true);
			Assert::AreEqual(GCode.HasWord('Z'), true);
		}

		TEST_METHOD(HasWord_WordDoesNotExist_ReturnsFalse)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			Assert::AreEqual(GCode.HasWord('M'), false);

			startAt = AddLine(&GCode, startAt);

			Assert::AreEqual(GCode.HasWord('C'), false);

			AddLine(&GCode, startAt);

			Assert::AreEqual(GCode.HasWord('C'), false);
			Assert::AreEqual(GCode.HasWord('H'), false);
		}

		TEST_METHOD(GetWordValue_ExistingWord_ReturnsValue)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			Assert::AreEqual((int)GCode.GetWordValue('G'), 1);
			Assert::AreEqual(GCode.GetWordValue('Z'), 0.0);

			startAt = AddLine(&GCode, startAt);

			Assert::AreEqual((int)GCode.GetWordValue('M'), 300);
			Assert::AreEqual(GCode.GetWordValue('S'), 125.0);

			AddLine(&GCode, startAt);

			Assert::AreEqual(GCode.GetWordValue('X'), 3.2);
			Assert::AreEqual(GCode.GetWordValue('Z'), 5.0);
		}
	};
}
