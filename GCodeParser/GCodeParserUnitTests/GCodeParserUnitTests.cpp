#include "pch.h"
#include "CppUnitTest.h"
#include "../GCodeParser/GCodeParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GCodeParserUnitTests
{
	TEST_CLASS(GCodeParserUnitTests)
	{
	public:
		TEST_METHOD(AddCharToLine_AddLines_ConfirmLines)
		{
			GCodeParser GCode = GCodeParser();

			char line[] = { 'G', '0', '1' , ' ', 'Z' , '0' , '.' , '0' , '\r', '\n', '\0' };

			bool completeLineIsAvailableToInterpret;
			int pointer = 0;
			while (line[pointer] != '\0')
			{
				completeLineIsAvailableToInterpret = GCode.AddCharToLine(line[pointer]);
				if (completeLineIsAvailableToInterpret)
					break;

				pointer++;
			}

			Assert::AreEqual(completeLineIsAvailableToInterpret, true);

			char resultLine[] = { 'G', '0', '1' , ' ', 'Z' , '0' , '.' , '0' };

			pointer = 0;
			while (GCode.line[pointer] != '\0')
			{
				Assert::AreEqual(GCode.line[pointer], resultLine[pointer]);

				pointer++;
			}

			char line2[] = { 'M', '3', '0', '0', ' ', 'S' , '1' , '2' , '5' , '\n', '\0' };

			pointer = 0;
			while (line2[pointer] != '\0')
			{
				completeLineIsAvailableToInterpret = GCode.AddCharToLine(line2[pointer]);

				pointer++;
			}

			Assert::AreEqual(completeLineIsAvailableToInterpret, true);

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

			Assert::Fail();
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

			char line[] = { 'G', '0', '1' , ' ', 'Z' , '0' , '.' , '0' , '\r', '\n', '\0' };

			int pointer = 0;
			while (line[pointer] != '\0')
			{
				GCode.AddCharToLine(line[pointer]);

				pointer++;
			}

			Assert::AreEqual(GCode.HasWord('G'), true);
		}

		TEST_METHOD(HasWord_WordDoesNotExist_ReturnsFalse)
		{
			GCodeParser GCode = GCodeParser();

			char line[] = { 'G', '0', '1' , ' ', 'Z' , '0' , '.' , '0' , '\r', '\n', '\0' };

			int pointer = 0;
			while (line[pointer] != '\0')
			{
				GCode.AddCharToLine(line[pointer]);

				pointer++;
			}

			Assert::AreEqual(GCode.HasWord('M'), false);
		}
	};
}
