#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstring>
#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#pragma comment(lib, "Wininet.lib") 
#ifdef UNICODE
#define A2W MultiByteToWideChar
#else
#define A2W MyStrcpy
#endif
using namespace std;
#define N 7 //版块数
#define COURES "https://ocw.swjtu.edu.cn/yethan/YouthIndex?setAction=courseInfo&courseid="//课程详情网址
#define VATUU "https://ocw.swjtu.edu.cn/yethan/YouthIndex?setAction=courseList&key5="//第二课堂版块网址
#define pName "第二课堂自动搜线上课"
#define AutoStartName "SecondClass"//设置开机启动的注册表名
#define BJTIME "https://www.citext.cn/GetTime.php"//北京时间网址
int MyStrcpy(int a, int b, const char *str1, int c, char *str2, int d);//对应MultiByteToWideChar，若不是UNICODE则直接copy字符串
string GetHttpText(string URL);//读取网页源代码
string GetMid(string text, string left, string right);//用两边文本取中间文本内容
void AutoStart();//设置/关闭开机自启动
//课程类型ID及对应课程类型名称
const string id[N] = { "61E92EF67418DC54","881A8F30F8EA0FAC","22251884ACC79046","36A0F9DAB7403C19","0E4BF4D36E232918","EE416E25D3991FB3","6C185FAF9C861783" };
const string name[N] = { "思想道德与修养","学术科技与创新创业","艺术体验与审美修养","文化沟通与交往能力","心理素质与身体素质","社会实践与志愿服务","社会工作与领导能力" };
vector<string> record;//用来记录已提醒过的课程
HWND hMain = GetConsoleWindow();
int main()
{
	HWND H = FindWindowA(NULL, pName);//判断本程序是否已启动，不用进程判断是因为这样方便
	if (H == NULL)
		H = FindWindowA(NULL, ("选择" + string(pName)).c_str());//Windows10的编辑模式会在标题前加“选择”
	if (H != NULL)
	{
		if (IsWindowVisible(H))
		{
			ShowWindow(hMain, SW_HIDE);
			AutoStart();
		}
		else
		{
			ShowWindow(H, SW_SHOW);
		}
		return 0;
	}
	if(GetTickCount()<100000)//视开机后小于100s启动程序为开机自启动，并隐藏控制台
		ShowWindow(hMain, 0);
	cout << "双开本程序可设置开机启动/关闭开机启动" << endl;
	cout << "开始自动搜索西南交通大学第二课堂线上课程，自动过滤选满和不在报名时间的课程..." << endl;
	for (SetConsoleTitleA(pName);; Sleep(100000))//设置控制台窗口标题，每100秒搜索一次，防止网络占用过多
	{
		//string BJTime = GetHttpText(BJTIME);//取北京时间（使用第二种过滤方法才需要）
		for (int i = 0; i < N; i++)
		{
			string	HttpText = GetHttpText(VATUU + id[i]);//取第二课堂对应版块网页内容
			int pos = 0;
			while ((pos = HttpText.find("endTime", pos + 1)) != -1)//遍历该版块第一页的所有课程
			{
				string place = GetMid(HttpText.substr(pos, 500), "times\">", " ");//取课程地点
				if (place.find("X") != -1 || place.find("x") != -1)//有教室号的课程必为线下，直接过滤
					continue;
				//第一种过滤方法，根据第二课堂的报名按钮颜色判断
				string color = GetMid(HttpText.substr(pos, 800), "background: ", ";color");
				if (color == "#BFBFBF")//如果报名按钮为灰色，则报名或不在报名时间，过滤
					continue;
				/*    第二种过滤方法
				string endTime = GetMid(HttpText.substr(pos, 50), " v=\"", "\"></span>");//取课程报名结束时间
				if (BJTime >= endTime)//若报名结束则跳过
					continue;
				string full = GetMid(HttpText.substr(pos, 1000), place, "/span>");//取出已报人数/总人数
				if (atoi(GetMid(full, " ", "/").c_str()) >= atoi(GetMid(full, "/", "<").c_str()))//再过滤出没选满的课程
					continue;
				*/
				if (place.find("线上") != -1 || place.find("QQ") != -1 || place.find("qq") != -1 || place.find("腾讯") != -1 || place.find("邮箱") != -1 || place.find("群") != -1)//过滤出线上课程
				{
					int pos_url_name = HttpText.rfind("getCourseInfo(\'", pos);//取课程网址及名称在文本的位置
					string CourseID = GetMid(HttpText.substr(pos_url_name, 200), "getCourseInfo(\'", "\')\">");//取课程号
					string CourseName = GetMid(HttpText.substr(pos_url_name, 200), "\')\">", "</p>");//取课程名称
					vector <string>::iterator it = find(record.begin(), record.end(), CourseName);//为了判断该课程是否提醒过
					if (it == record.end())//最后过滤出没提醒过的课程
					{
						string out = "<" + name[i] + ">版块的<" + CourseName + ">\n";//输出符合线上条件的版块和课程名称
						cout << out;
						if (MessageBoxA(hMain, (out + "本次运行不再提醒，是否查看详情？").c_str(), "选课提醒", MB_YESNO | MB_ICONQUESTION) == IDYES)//弹窗提醒
						{
							ShellExecuteA(NULL, NULL, (COURES + CourseID).c_str(), NULL, NULL, SW_NORMAL);//点击是，则打开第二课堂对应课程详情网址
						}
						record.push_back(CourseName);//记录已经提醒过的课程
					}
				}
			}
		}
	}
	return 0;
}
string GetMid(string text, string left, string right)
{
	int l = text.find(left);
	int pos = l + left.length();
	int r = text.substr(pos).find(right);
	if (l == -1 || r == -1)
		return "";
	return text.substr(pos, r);
}
string GetHttpText(string URL)
{
	TCHAR url[1024];
	string text;
	A2W(CP_ACP, 0, URL.c_str(), URL.length() + 1, url, sizeof(url) / sizeof(url[0]));
	HINTERNET hSession = InternetOpen(_T("UrlTest"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession != NULL)
	{
		HINTERNET hHttp;
		hHttp = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
		if (hHttp != NULL)
		{
			char Str[1024];
			ULONG Number = 1;
			while (Number > 0)
			{
				InternetReadFile(hHttp, Str, 1023, &Number);
				Str[Number] = '\0';
				text += Str;
			}
			InternetCloseHandle(hHttp);
			hHttp = NULL;
		}
		InternetCloseHandle(hSession);
		hSession = NULL;
	}
	int widesize = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
	vector<WCHAR> retw(widesize);
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &retw[0], widesize);
	wstring wstr(&retw[0]);
	int asciisize = WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	vector<char> reta(asciisize);
	WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, &reta[0], asciisize, NULL, NULL);
	return string(&reta[0]);
}
int MyStrcpy(int a, int b, const char *str1, int c, char *str2, int d)
{
	return (int)strcpy(str2, str1);
}
void AutoStart()
{
	const char PATH[MAX_PATH] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"; //注册表路径
	HKEY hKey;
	TCHAR Path[MAX_PATH] = {};
	A2W(CP_ACP, 0, PATH, strlen(PATH) + 1, Path, sizeof(Path) / sizeof(Path[0]));
	LSTATUS SUCCESS = RegOpenKeyEx(HKEY_CURRENT_USER, Path, 0, KEY_ALL_ACCESS, &hKey);
	if (SUCCESS == ERROR_SUCCESS) //打开启动项    
	{
		TCHAR strExeFullDir[MAX_PATH];
		GetModuleFileName(NULL, strExeFullDir, MAX_PATH);//得到本程序自身的全路径
		TCHAR strDir[MAX_PATH] = {};
		DWORD nLength = MAX_PATH;
		TCHAR keyName[MAX_PATH] = {};
		A2W(CP_ACP, 0, AutoStartName, strlen(PATH) + 1, keyName, sizeof(keyName) / sizeof(keyName[0]));
		long result = RegGetValue(hKey, NULL, keyName, RRF_RT_REG_SZ, 0, strDir, &nLength);
		if (result != ERROR_SUCCESS || _tcscmp(strExeFullDir, strDir) != 0)//判断注册表项是否已经存在
		{
			if (RegSetValueEx(hKey, keyName, 0, REG_SZ, (LPBYTE)strExeFullDir, (lstrlen(strExeFullDir) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
				MessageBoxA(NULL, "已开启开机自启动", "提示", MB_ICONQUESTION);
			else
				MessageBoxA(NULL, "开机自启动开启失败", "提示", MB_ICONERROR);
		}
		else
		{
			if (RegDeleteValue(hKey, keyName) == ERROR_SUCCESS)
				MessageBoxA(NULL, "已关闭开机自启动", "提示", MB_ICONQUESTION);
			else
				MessageBoxA(NULL, "开机自启动开启失败", "提示", MB_ICONERROR);
		}
	}
	else
	{
		MessageBoxA(NULL, "开机自启动开启失败", "提示", MB_ICONERROR);
	}
	RegCloseKey(hKey);
}
