#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <io.h>

#define READSIZE 102400

unsigned char key[256];
size_t encodelen;
unsigned char* buff;
int EncodeOp = 0;
unsigned char sbuff[2][8];//
unsigned char filehead[] = { 0xE8, 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3, 0xC3 };
int EncodeFile(const char* dirpath, const char* filename)
{
	char filepath[MAX_PATH];
	char temppath[MAX_PATH];

	strcpy(filepath, dirpath);
	strcat(filepath, "\\");
	strcat(filepath, filename);
	strcpy(temppath, filepath);
	strcat(temppath, ".c3");

	if (!rename(filepath, temppath))
	{
		FILE* fp = fopen(temppath, "rb");
		if (fp)
		{
			FILE* fencode = fopen(filepath, "wb");
			if (fencode)
			{
				//unsigned char filehead[] = { 0xE8, 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3, 0xC3 };
				//fwrite(filehead, 1, sizeof(filehead), fencode);

				size_t szRead = 0;
				int count = 0;
				while (szRead = fread(buff, 1, READSIZE, fp))
				{
					while (count < szRead)
					{
						buff[count] ^= key[count % encodelen];
						count++;
					}
					count = 0;
					fwrite(buff, 1, szRead, fencode);
				}
				fclose(fencode);
			}

			fclose(fp);
		}
		remove(temppath);
	}
	return 1;
}


int EncodeAndDecodeFile(const char* dirpath, const char* filename)
{
	char filepath[MAX_PATH];
	int flag = 0;
	INT64 i = 0;
	INT64 bufflen = 0;
	INT64 szRead = 0;
	//size_t szWrite = 0;
	strcpy(filepath, dirpath);
	strcat(filepath, "\\");
	strcat(filepath, filename);
	FILE* fp = fopen(filepath, "rb+");
	if (fp)
	{
		if (EncodeOp)
		{
			fread(sbuff, 1, 8, fp);
			if ((*(size_t*)sbuff != 0xC3C390909090E9E8))
			{
				_fseeki64(fp, 0, SEEK_SET);
				while (szRead = fread(buff, 1, READSIZE, fp))
				{
					if ((bufflen = szRead - READSIZE + 8) > 0)
					{
						i = 0;
						while (i < bufflen)
						{
							sbuff[!flag][i] = buff[READSIZE - 8 + i] ^ key[(READSIZE - 8 + i) % encodelen];
							i++;
						}
						i = READSIZE - 1;
						while (i > 7)
						{
							buff[i] = buff[i - 8] ^ key[(i - 8) % encodelen];
							i--;
						}
						i = 0;
						while (i < 8)
						{
							buff[i] = sbuff[flag][i];
							i++;
						}
						flag = !flag;
						_fseeki64(fp, -szRead, SEEK_CUR);
						fwrite(buff, 1, READSIZE, fp);
					}
					else
					{
						i = szRead + 7;
						while (i > 7)
						{
							buff[i] = buff[i - 8] ^ key[(i - 8) % encodelen];
							i--;
						}
						i = 0;
						while (i < 8)
						{
							buff[i] = sbuff[flag][i];
							i++;
						}
						_fseeki64(fp, -szRead, SEEK_CUR);
						fwrite(buff, 1, szRead + 8, fp);
						break;
					}
				}
				_fseeki64(fp, 0, SEEK_SET);
				fwrite(filehead, 1, 8, fp);
			}
			fclose(fp);
		}
		else
		{
			fread(sbuff, 1, 8, fp);
			if ((*(size_t*)sbuff == 0xC3C390909090E9E8))
			{
				char filepathback[MAX_PATH];
				strcpy(filepathback, filepath);
				strcat(filepath, ".tmp");
		
				FILE* tmp = fopen(filepath, "wb");
				if (tmp)
				{
					while (szRead = fread(buff, 1, READSIZE, fp))
					{
						i = 0;
						while (i < szRead)
						{
							buff[i] ^= key[i % encodelen];
							i++;
						}
						fwrite(buff, 1, szRead, tmp);
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
			EncodeAndDecodeFile(dir, findData.name);
		}
	} while (_findnext64(handle, &findData) == 0);

	_findclose(handle);    // 关闭搜索句柄
	return 1;
}

int main(intptr_t argc, char** argv)
{
	scanf("%d", &EncodeOp);
	scanf("%s", &key);
	encodelen = strlen((const char*)key);
	buff = (unsigned char*)malloc(READSIZE);
	printf("begin\n");

	if (argv[1])
	{
		printf("%s\n", argv[1]);
		ListFiles(argv[1]);
	}
	else
	{
		char dir[] = "C:\\Users\\11042\\Desktop\\新建文件夹 (2)";
		ListFiles(dir);
	}

	//long long szRead;
	//char buff[4096*40];
	//FILE* fp = fopen("D:\\EVA\\[大体积]Evangelion.3.0+1.01.Thrice.Upon.a.Time.2021.中文字幕.WEBrip.AAC.1080p.x264-VINEnc.mp4", "rb");
	//if (fp)
	//{
	//	FILE* fpp = fopen("C:\\Users\\11042\\Desktop\\新建文件夹 (2)\\1323.mp4", "wb");
	//	if (fpp)
	//	{
	//		while (szRead = fread(buff, 1, 4096*40, fp))
	//		{
	//			fwrite(buff, 1, szRead, fpp);
	//		}
	//		fclose(fpp);
	//	}
	//	fclose(fp);
	//}

	return 0;
}

