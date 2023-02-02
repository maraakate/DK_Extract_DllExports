// DK_Extract_DllExports.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

char rootPath[MAX_PATH];
char outFile[MAX_PATH];

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
		if (!strncmp(line, DllExportToken, sizeof(DllExportToken) - 1) && strncmp(prevLine, HardLinked, sizeof(HardLinked) -1))
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
		snprintf(foundFile, sizeof(foundFile), "%s\\%s", rootPath, findData.cFileName);
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
	char searchPath[MAX_PATH] = { 0 };

	if (argc != 3)
	{
		printf("You must pass the root path and out file.\n");
		printf("\ni.e. dk_extract_dllexports.exe C:\\proj\\daikatana-1.3\\dlls\\weapons C:\\proj\\daikatana-1.3\\dlls\\weapons\\exports.txt\n");
		return -1;
	}

	strncpy(rootPath, argv[1], sizeof(rootPath) - 1);
	strncpy(outFile, argv[2], sizeof(outFile) - 1);

	fout = fopen(outFile, "w+");
	if (!fout)
	{
		printf ("Failed to create file '%s'\n", outFile);
		return -2;
	}

	snprintf(searchPath, sizeof(searchPath), "%s\\*.h", rootPath);
	SearchPathForExports(searchPath);

	snprintf(searchPath, sizeof(searchPath), "%s\\*.cpp", rootPath);
	SearchPathForExports(searchPath);

	fclose(fout);

	return 0;
}
