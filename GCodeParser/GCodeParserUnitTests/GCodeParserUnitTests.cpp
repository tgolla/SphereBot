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
			char lines[] = { 'G', '0', '1' , ' ', 'Z' , '0' , '.' , '0' , '\r', '\n', 
				'M', '3', '0', '0', ' ', 'S', '1', '2', '5', '\n', '\0' };

			bool completeLineIsAvailableToInterpret;
			while (lines[startAt] != '\0')
			{
				completeLineIsAvailableToInterpret = gcode->AddCharToLine(lines[startAt]);
				if (completeLineIsAvailableToInterpret)
					break;

				startAt++;
			}

			return startAt;
		}


		TEST_METHOD(AddCharToLine_AddLines_ConfirmLines)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			char resultLine[] = { 'G', '0', '1' , ' ', 'Z' , '0' , '.' , '0' };

			int pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.line[pointer], resultLine[pointer]);

				pointer++;
			}

			AddLine(&GCode, startAt);

			char resultLine2[] = { 'M', '3', '0', '0', ' ', 'S' , '1' , '2' , '5' };

			pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.line[pointer], resultLine2[pointer]);

				pointer++;
			}
		}

		TEST_METHOD(AddCharToLine_AddLines_ConfirmCodeBlock)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			char resultCodeBlock[] = { 'G', '0', '1' , 'Z' , '0' , '.' , '0' };

			int pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.codeBlock[pointer], resultCodeBlock[pointer]);

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
		
		TEST_METHOD(HasWord_NoCodeBlock_ReturnsFalse)
		{
			GCodeParser GCode = GCodeParser();

			Assert::AreEqual(GCode.HasWord('G'), false);
		}

		TEST_METHOD(HasWord_WordExist_ReturnsTrue)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			Assert::AreEqual(GCode.HasWord('G'), true);
		}

		TEST_METHOD(HasWord_WordDoesNotExist_ReturnsFalse)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			Assert::AreEqual(GCode.HasWord('M'), false);
		}

		TEST_METHOD(GetWordValue_ExistingWord_ReturnsValue)
		{
			GCodeParser GCode = GCodeParser();

			int startAt = AddLine(&GCode, 0);

			Assert::AreEqual((int)GCode.GetWordValue('G'), 1);
		}
	};
}
