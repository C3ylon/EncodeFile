#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <io.h>

//PathIsDirectory
#include<Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define READSIZE 1048576
unsigned char key[256];
unsigned char shift[8];
unsigned char reshift[8];
size_t* payload = (size_t*)((INT32*)key + 1);

unsigned char* buff;
int EncodeOp = 0;
unsigned char sbuff[8];//
unsigned char filehead[] = { 0xE8, 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3, 0x00 };
size_t temp = 0;


int EncodeAndDecodeFile(const char* dirpath, const char* filename, int isfolder)
{
	char filepath[MAX_PATH];
	char filepathback[MAX_PATH];
	int flag = 0;
	size_t i = 0;
	int j = 0;
	size_t bufflen = 0;
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
		if (EncodeOp)
		{
			fread(sbuff, 1, 8, fp);
			if ((*(size_t*)sbuff & 0xFFFFFFFFFFFFFF) != 0xC390909090E9E8)
			{
				_fseeki64(fp, 0, SEEK_END);
				szFile = _ftelli64(fp);
				align = szFile % 8;
				if (align)
				{
					fwrite(&alignbuff, 1, 8 - align, fp);
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
							temp = *((size_t*)buff + i);
							j = 0;
							while (j < 8)
							{
								temp = (temp << shift[j]) | (temp >> (64 - shift[j]));
								temp ^= *payload;
								j++;
							}
							*((size_t*)buff + i) = temp;
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
		else
		{
			fread(sbuff, 1, 8, fp);
			if ((*(size_t*)sbuff & 0xFFFFFFFFFFFFFF) == 0xC390909090E9E8)
			{
				align = *((unsigned char*)sbuff + 7) & 0xF;
				char filepathback[MAX_PATH];
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
							temp = *((size_t*)buff + i);
							j = 0;
							while (j < 8)
							{
								temp ^= *payload;
								temp = (temp >> reshift[j]) | (temp << (64 - reshift[j]));
								j++;
							}
							*((size_t*)buff + i) = temp;
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
		strcat(dirNew, "\\*.*");    // 在目录后面加上"\\*.*"进行第一次搜索
		__finddata64_t findData;
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
				//printf("%s\t<dir>\n", dirNew);

				printf("[FILE]%s\n", findData.name);

				memset(dirNew, 0, MAX_PATH);
				strcpy(dirNew, dir);
				strcat(dirNew, "\\");
				strcat(dirNew, findData.name);
				// 在目录后面加上"\\"和搜索到的目录名进行下一次搜索


				ListFiles(dirNew);
			}
			else
			{
				printf("%s\t%lld bytes\n", findData.name, findData.size);
				//EncodeFile(dir, findData.name);
				EncodeAndDecodeFile(dir, findData.name, TRUE);
			}
		} while (_findnext64(handle, &findData) == 0);

		_findclose(handle);    // 关闭搜索句柄
	}
	return 1;
}

int main(intptr_t argc, char** argv)
{
	scanf("%d", &EncodeOp);
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
		printf("begin\n");
		printf("%s\n", argv[1]);
		ListFiles(argv[1]);
	}
	else
	{
		char dir[MAX_PATH];
		scanf("%s", &dir);
		printf("begin\n");
		ListFiles(dir);
	}
	return 0;
}

