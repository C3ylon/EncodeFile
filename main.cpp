#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include<time.h>

#define READSIZE 102400

unsigned char key[256];
size_t encodelen;
unsigned char* buff;
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

int ListFiles(const char* dir)
{
	char dirNew[MAX_PATH];
	strcpy(dirNew, dir);
	strcat(dirNew, "\\*.*");    // ��Ŀ¼�������"\\*.*"���е�һ������
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
			// ��Ŀ¼�������"\\"����������Ŀ¼��������һ������


			ListFiles(dirNew);
		}
		else
		{
			printf("%s\t%lld bytes\n", findData.name, findData.size);
			EncodeFile(dir, findData.name);
		}
	} while (_findnext64(handle, &findData) == 0);

	_findclose(handle);    // �ر��������
	return 1;
}

int main(intptr_t argc, char** argv)
{
	time_t start, end;
	start = time(NULL);
	
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
		char dir[] = "C:\\Users\\11042\\Desktop\\�½��ļ��� (2)";
		ListFiles(dir);
	}

	//long long szRead;
	//char buff[4096*40];
	//FILE* fp = fopen("D:\\EVA\\[�����]Evangelion.3.0+1.01.Thrice.Upon.a.Time.2021.������Ļ.WEBrip.AAC.1080p.x264-VINEnc.mp4", "rb");
	//if (fp)
	//{
	//	FILE* fpp = fopen("C:\\Users\\11042\\Desktop\\�½��ļ��� (2)\\1323.mp4", "wb");
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

	end = time(NULL);

	printf("time = %lf��\n", difftime(end, start));

	system("pause");
	return 0;
}

