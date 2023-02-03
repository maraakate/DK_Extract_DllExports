// BSD 2-Clause License
//
// Copyright (c) 2023, Frank Sapone
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#ifndef _MAX_PATH
#define _MAX_PATH	PATH_MAX
#endif // !_MAX_PATH
#endif // _WIN32

char srcPath[_MAX_PATH];
char outFile[_MAX_PATH];

FILE *fout;

const char DllExportToken[] = "DllExport";
const char HardLinked[] = "#ifdef GAME_HARD_LINKED";

static void ParseFile (char *filename)
{
	FILE *f = fopen(filename, "r");
	char line[2048] = { 0 };
	char prevLine[2048] = { 0 };
	char delim[] = " \t(";
	if (!f)
	{
		printf("Failed to open file '%s'\n", filename);
		return;
	}

	while (fgets(line, sizeof(line), f))
	{
		if (!strncmp(line, DllExportToken, sizeof(DllExportToken) - 1) && (strncmp(prevLine, HardLinked, sizeof(HardLinked) -1) != 0))
		{
			int x = 0;
			char *p = line;
			p += sizeof(DllExportToken);
			while (p[0] == ' ')
				p++;

			char *token = strtok(p, delim);
			while (token)
			{
				if (x == 1)
				{
					char buf[2048] = { 0 };

					while(token[0] == '*' || token[0] == '&')
						token++;

					snprintf(buf, sizeof(buf), "_%s\n", token);
					printf("%s", buf);
					fputs(buf, fout);
					break;
				}

				x++;
				token = strtok(NULL, delim);
			}
		}

		/* FS: Don't export #ifdef GAME_HARD_LINKED stuff.  Mostly for dll_Entry. */
		strcpy(prevLine, line);
	}

	fclose(f);
}

#ifdef _WIN32
static void SearchPathForExports (const char *path)
{
	HANDLE dirHandle;
	WIN32_FIND_DATA findData;
	int found = 1;

	dirHandle = FindFirstFile(path, &findData);

	if (dirHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	while (found)
	{
		char foundFile[MAX_PATH];
		//printf("Found %s\n", findData.cFileName);
		snprintf(foundFile, sizeof(foundFile), "%s\\%s", srcPath, findData.cFileName);
		ParseFile(foundFile);
		found = FindNextFile(dirHandle, &findData);
	}

	FindClose(dirHandle);
}
#else
static void SearchPathForExports (const char *path)
{
	printf("ERROR: Not implemented on this platform!\n");
}
#endif

int main(int argc, char *argv[])
{
	char searchPath[_MAX_PATH] = { 0 };

	if (argc != 3)
	{
		printf("You must specifiy the source path and out file.\n");
		printf("\ni.e. dk_extract_dllexports.exe C:\\proj\\daikatana-1.3\\dlls\\weapons C:\\proj\\daikatana-1.3\\dlls\\weapons\\exports.txt\n");
		return -1;
	}

	strncpy(srcPath, argv[1], sizeof(srcPath) - 1);
	strncpy(outFile, argv[2], sizeof(outFile) - 1);

	fout = fopen(outFile, "w+");
	if (!fout)
	{
		printf ("Failed to create file '%s'\n", outFile);
		return -2;
	}

	snprintf(searchPath, sizeof(searchPath), "%s\\*.h", srcPath);
	SearchPathForExports(searchPath);

	snprintf(searchPath, sizeof(searchPath), "%s\\*.cpp", srcPath);
	SearchPathForExports(searchPath);

	fclose(fout);

	return 0;
}
