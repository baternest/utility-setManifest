// SetManifest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include"tinyxml2.h"
//using namespace std;
//using namespace tinyxml2;
//#include <clocale>
//#include <locale>
//#include <string>
//#include <vector>
#include <codecvt>

#include "Windows.h"
#include <Shlwapi.h>

#pragma comment (lib, "Shlwapi.lib")
typedef std::basic_string<TCHAR> TString;

TString GetExeFilePath(void)
{
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		return TString();
	}

	PathRemoveFileSpec(szPath);
	PathAddBackslash(szPath);

	return TString(szPath);
}

void setLevel(tinyxml2::XMLDocument*& doc, int level)
{
	tinyxml2::XMLElement* assemblyElement = doc->FirstChildElement("assembly");
	tinyxml2::XMLElement* trustInfoElement = assemblyElement->FirstChildElement("trustInfo");
	tinyxml2::XMLElement* securityElement = trustInfoElement->FirstChildElement("security");
	tinyxml2::XMLElement* requestedPrivilegesElement = securityElement->FirstChildElement("requestedPrivileges");
	tinyxml2::XMLElement* requestedExecutionLevelElement = requestedPrivilegesElement->FirstChildElement("requestedExecutionLevel");

	switch (level)
	{
	case 1:
		requestedExecutionLevelElement->SetAttribute("level", "highestAvailable");
		break;
	case 2:
		requestedExecutionLevelElement->SetAttribute("level", "requireAdministrator");
		break;
	case 0:
	default:
		requestedExecutionLevelElement->SetAttribute("level", "asInvoker");
		break;
	}
}

void addWinBlueSupport(tinyxml2::XMLDocument*& doc)
{
	tinyxml2::XMLElement* assemblyElement = doc->FirstChildElement("assembly");
	tinyxml2::XMLElement* compatibilityElement = assemblyElement->FirstChildElement("compatibility");

	if (NULL != compatibilityElement)
	{
		assemblyElement->DeleteChild(compatibilityElement);
	}

	compatibilityElement = doc->NewElement("compatibility");
	compatibilityElement->SetAttribute("xmlns", "urn:schemas-microsoft-com:compatibility.v1");
	tinyxml2::XMLNode* node = assemblyElement->InsertEndChild(compatibilityElement);

	tinyxml2::XMLElement* applicationElement = doc->NewElement("application");
	node = node->InsertEndChild(applicationElement);

	tinyxml2::XMLElement* supportedOSElement;
	// <!-- Windows 8.1 -->
	supportedOSElement = doc->NewElement("supportedOS");
	supportedOSElement->SetAttribute("Id", "{1f676c76-80e1-4239-95bb-83d0f6d0da78}");	
	node->InsertEndChild(supportedOSElement);

	// <!-- Windows 7 -->
	supportedOSElement = doc->NewElement("supportedOS");
	supportedOSElement->SetAttribute("Id", "{35138b9a-5d96-4fbd-8e2d-a2440225f93a}");
	node->InsertEndChild(supportedOSElement);

	// <!-- Windows 8 -->
	supportedOSElement = doc->NewElement("supportedOS");
	supportedOSElement->SetAttribute("Id", "{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}");
	node->InsertEndChild(supportedOSElement);
}

std::string ws2s(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::wstring s2ws(const std::string& str)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

void ExtractManifest(std::string exeFilePath)
{
	std::wstring wsFilePath = s2ws(exeFilePath);
	std::wstring parameters = TEXT("-inputresource:\"");
	parameters += wsFilePath;
	parameters += TEXT("\" -out:\"");
	parameters += wsFilePath;
	parameters += TEXT(".manifest\"");

	TString mtExe = TEXT("\"");
	mtExe += GetExeFilePath();
	mtExe += TEXT("mt.exe\"");

	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = mtExe.c_str();
	ShExecInfo.lpParameters = parameters.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
}

void UpdateManifest(std::string exeFilePath)
{
	std::wstring wsFilePath = s2ws(exeFilePath);
	std::wstring parameters = TEXT("-manifest \"");
	parameters += wsFilePath;
	parameters += TEXT(".manifest\" -outputresource:\"");
	parameters += wsFilePath;
	parameters += TEXT("\"");

	TString mtExe = TEXT("\"");
	mtExe += GetExeFilePath();
	mtExe += TEXT("mt.exe\"");

	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = mtExe.c_str();
	ShExecInfo.lpParameters = parameters.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
}

int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR* filename = NULL;
	TCHAR* level = NULL;
	bool bWinBlue = false;

	if (argc < 2)
	{
		_tprintf(TEXT("Usage:\n"));
		_tprintf(TEXT("-----\n"));
		_tprintf(TEXT("SetManifest.exe\n"));
		//_tprintf(TEXT("  [ -f (file) manifest file name]\n"));
		_tprintf(TEXT("  [ -f (file) Executable file name]\n"));
		_tprintf(TEXT("  [ -l (level) requestedExecutionLevel]\n"));
		_tprintf(TEXT("               0: asInvoker\n"));
		_tprintf(TEXT("               1: highestAvailable\n"));
		_tprintf(TEXT("               2: requireAdministrator\n"));
		_tprintf(TEXT("  [ -w (Windows 8.1/WinBlue supported)]\n"));
		_tprintf(TEXT("\n"));
		_tprintf(TEXT("Sample:\n"));
		_tprintf(TEXT("-----\n"));
		_tprintf(TEXT("> To set manifest with requireAdministrator level and Windows 8.1 supported.\n"));
		_tprintf(TEXT("SetManifest.exe -f my.exe.manifest -l 2 -w\n"));

		return 1;
	}

	int count;

	for (count = 1; count < argc; count++)
	{
		TCHAR* arg = argv[count];

		if (_tcsicmp(argv[count], _TEXT("-f")) == 0)
		{
			count += 1;
			filename = argv[count];
			_tprintf(TEXT("filename: %s\n"), filename);
		}
		else if (_tcsncicmp(argv[count], _TEXT("-f"), 2) == 0)
		{
			filename = argv[count] + 2;
			_tprintf(TEXT("filename: %s\n"), filename);
		}

		if (_tcsicmp(argv[count], _TEXT("-l")) == 0)
		{
			count += 1;
			level = argv[count];
			_tprintf(TEXT("level: %s\n"), level);
		}
		else if (_tcsncicmp(argv[count], _TEXT("-l"), 2) == 0)
		{
			level = argv[count] + 2;
			_tprintf(TEXT("level: %s\n"), level);
		}

		if (_tcsicmp(argv[count], _TEXT("-w")) == 0)
		{
			bWinBlue = true;
		}
	}
	
	TString exeFilePath = GetExeFilePath();
	TString mtFile = exeFilePath + TEXT("mt.exe");

	if (!PathFileExists(mtFile.c_str()))
	{
		_tprintf(TEXT("Manifest Tool not exist!\nPlease copy mt.exe to %s"), exeFilePath.c_str());
		return -1;
	}

	if (!PathFileExists(filename))
	{
		exeFilePath += filename;
		filename = (TCHAR*)exeFilePath.c_str();
	}

	if (!PathFileExists(filename))
	{
		_tprintf(TEXT("Executable file: %s not exist!\n"), filename);
		return -1;
	}


	std::string exeFile = ws2s(filename);
	std::string manifest = exeFile + ".manifest";

	ExtractManifest(exeFile);

	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();

	if (tinyxml2::XML_NO_ERROR != doc->LoadFile(manifest.c_str()))
	{
		_tprintf(TEXT("Load XML failed!\n"));
		goto ErrorExit;
	}

	if (level != NULL)
	{
		setLevel(doc, _tstoi(level));
	}

	if (bWinBlue)
	{
		addWinBlueSupport(doc);
	}

	if (tinyxml2::XML_NO_ERROR != doc->SaveFile(manifest.c_str()))
	{
		_tprintf(TEXT("Save XML file failed!\n"));
		goto ErrorExit;
	}
	
	delete doc;

	UpdateManifest(exeFile);
	DeleteFile(s2ws(manifest).c_str());
	_tprintf(TEXT("Set manifest successful!\n"));
	return 0;

ErrorExit:
	delete doc;
	return -1;
}