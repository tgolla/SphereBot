#include "pch.h"
#include "CppUnitTest.h"
#include "../GCodeParser/GCodeParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GCodeParserUnitTests
{
	TEST_CLASS(GCodeParserUnitTests)
	{
	public:

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
