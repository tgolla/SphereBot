/*
EEPROMTyped.h - EEPROM library
Copyright (c) 2014 Terence F. Golla.  All right reserved.

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROMTyped_h
#define EEPROMTyped_h

#include <Arduino.h>
#include <EEPROM.h>

struct EEPROMTypedClass
{
	// Note: Implementation of templated functions must reside in the header file for the compiler
	// to create the correct type instantiations when building main, hense preventing error messages
	// like... undefined reference to `int EEPROMTyped::read<int>(int, int&)'
	// ref: https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

	// Returns the byte size of the variable.
	template <class T>
	int sizeOf(T &variable)
	{
		return sizeof(variable);
	}

	// Writes the variable value to the EEPROM memory address.
	template <class T>
	int write(int address, const T &variable)
	{
		const byte *p = (const byte *)(const void *)&variable;
		unsigned int i;
		for (i = 0; i < sizeof(variable); i++)
		{
			const byte b = *p;
			if (EEPROM.read(address) != b)
				EEPROM.write(address++, b), ++p;
			else
				address++, p++;
		}
		return i;
	}

	// Reads the variables value from the EEPROM memory address.
	template <class T>
	int read(int address, T &variable)
	{
		byte *p = (byte *)(void *)&variable;
		unsigned int i;
		for (i = 0; i < sizeof(variable); i++)
			*p++ = EEPROM.read(address++);
		return i; // Returns the variable size in bytes.
	}
};

static EEPROMTypedClass EEPROMTyped;

#endif
