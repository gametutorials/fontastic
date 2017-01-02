#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "GT_Util.h"

// Makes "str" lowercase
void Lowercase(char *str)
{
	if(str)
	{
		for(uint i = 0; i < strlen(str); ++i)
		{
			if(isalpha(str[i]))
				str[i] = tolower(str[i]);
		}
	}
}

// Makes "str" uppercase
void Uppercase(char *str)
{
	if(str)
	{
		for(uint i = 0; i < strlen(str); ++i)
		{
			if(isalpha(str[i]))
				str[i] = toupper(str[i]);
		}
	}
}

void OutputError(const char *err, ...)
{
#ifdef _DEBUG
	char buff[256] = {0};
	va_list args;

    va_start(args, err);
    int count = vsprintf(buff, err, args);
    va_end(args);
    
    if(count > 0)
    	OutputDebugString(buff);
#endif
}
   

	