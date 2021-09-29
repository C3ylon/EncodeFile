//#define _CRT_SECURE_NO_WARNINGS
//gcc .\filecrypto.c -o filecrypto -O3 -lshlwapi
#include <Windows.h>
#include <stdio.h>
#include <io.h>
//PathIsDirectory
#include<Shlwapi.h>
//if compile in vs2019, add next line:
//#pragma comment(lib, "shlwapi.lib")

#define READSIZE 1048576

unsigned char key[256];
size_t* payload = (size_t*)((INT32*)key + 1);
unsigned char shift[8];
unsigned char reshift[8];
unsigned char* buff;
int if_encode = 0;
unsigned char filehead[] = { 0xE8, 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3, 0xC0 };
size_t transbuff = 0;

int EncodeAndDecodeFile(const char* dirpath, const char* filename, int isfolder)
{
	char filepath[MAX_PATH];
	char filepathback[MAX_PATH];
	int flag = 0;
	size_t i = 0;
	int j = 0;
	size_t szRead = 0;
	size_t szFile = 0;
	unsigned char align = 0;
	size_t alignbuff = 0xFFFFFFFFFFFFFFFF;

	strcpy(filepath, dirpath);
	if (isfolder)
	{
		strcat(filepath, "\\");
		strcat(filepath, filename);
	}

	FILE* fp = fopen(filepath, "rb+");
	if (fp)
	{
		if (if_encode)
		{
			fread(buff, 1, 8, fp);
			if ((*(size_t*)buff & 0xFFFFFFFFFFFFFF) != 0xC390909090E9E8)
			{
				_fseeki64(fp, 0, SEEK_END);
				szFile = _ftelli64(fp);
				align = szFile % 8;
				if (align)
				{
					fwrite(&alignbuff, 1, 8 - align, fp);
					filehead[7] &= 0xF0;
					filehead[7] ^= align;
				}
				_fseeki64(fp, 0, SEEK_SET);
				strcpy(filepathback, filepath);
				strcat(filepath, ".tmp");

				FILE* tmp = fopen(filepath, "wb");
				if (tmp)
				{
					fwrite(filehead, 1, 8, tmp);
					while (szRead = fread(buff, 1, READSIZE, fp))
					{
						i = 0;
						szRead = szRead >> 3;
						while (i < szRead)
						{
							transbuff = *((size_t*)buff + i);
							j = 0;
							while (j < 8)
							{
								transbuff = (transbuff << shift[j]) | (transbuff >> (64 - shift[j]));
								transbuff ^= *payload;
								j++;
							}
							*((size_t*)buff + i) = transbuff;
							i++;
						}
						fwrite(buff, 8, szRead, tmp);
					}
					fclose(tmp);
				}
				fclose(fp);
				remove(filepathback);
				rename(filepath, filepathback);
				return 1;
			}
			fclose(fp);
		}
		else//decode
		{
			fread(buff, 1, 8, fp);
			if ((*(size_t*)buff & 0xFFFFFFFFFFFFFF) == 0xC390909090E9E8)
			{
				align = *((unsigned char*)buff + 7) & 0xF;
				strcpy(filepathback, filepath);
				strcat(filepath, ".tmp");
				FILE* tmp = fopen(filepath, "wb");
				if (tmp)
				{
					while (szRead = fread(buff, 1, READSIZE, fp))
					{
						i = 0;
						szRead = szRead >> 3;
						while (i < szRead)
						{
							transbuff = *((size_t*)buff + i);
							j = 0;
							while (j < 8)
							{
								transbuff ^= *payload;
								transbuff = (transbuff >> reshift[j]) | (transbuff << (64 - reshift[j]));
								j++;
							}
							*((size_t*)buff + i) = transbuff;
							i++;
						}
						szRead = szRead << 3;
						if(feof(fp) && align)
						{
							fwrite(buff, 1, szRead - 8 + align, tmp);
						}
						else
						{
							fwrite(buff, 1, szRead, tmp);
						}
					}
					fclose(tmp);
				}
				fclose(fp);
				remove(filepathback);
				rename(filepath, filepathback);
				return 1;
			}
			fclose(fp);
		}
	}
	return 1;
}

int ListFiles(const char* dir)
{
	char dirNew[MAX_PATH];
	strcpy(dirNew, dir);
	if (!PathIsDirectoryA(dir))
	{
		EncodeAndDecodeFile(dir, "", FALSE);
	}
	else
	{
		strcat(dirNew, "\\*.*");
		struct __finddata64_t findData;
		intptr_t handle = _findfirst64(dirNew, &findData);
		if (handle == -1)
		{
			printf("[!]find path fail\n");
			return 0;
		}
		do
		{
			if (findData.attrib & _A_SUBDIR)
			{
				if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
					continue;
				printf("[FOLDER]%s\n", findData.name);
				memset(dirNew, 0, MAX_PATH);
				strcpy(dirNew, dir);
				strcat(dirNew, "\\");
				strcat(dirNew, findData.name);
				ListFiles(dirNew);
			}
			else
			{
				printf("[FILE]%s\t%lld bytes\n", findData.name, findData.size);
				EncodeAndDecodeFile(dir, findData.name, TRUE);
			}
		} while (_findnext64(handle, &findData) == 0);
		_findclose(handle);
	}
	return 1;
}

int main(int argc, char** argv)
{
	scanf("%d", &if_encode);
	scanf("%s", &key);

	int i = 0;
	while (i < 4)
	{
		reshift[7 - 2 * i] = shift[2 * i] = key[i] >> 4;
		reshift[6 - 2 * i] = shift[2 * i + 1] = key[i] & 0xF;
		i++;
	}

	buff = (unsigned char*)malloc(READSIZE);
	if (argv[1])
	{
		printf("[*]BEGIN\n");
		printf("[FILE_PATH]%s\n", argv[1]);
		ListFiles(argv[1]);
	}
	else
	{
		char dir[MAX_PATH];
		scanf("%s", &dir);
		printf("[*]BEGIN\n");
		ListFiles(dir);
	}
	return 0;
}
