/**
 * FileName: Helper.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass.
 * DWMBlurGlass is free software: you can redistribute it and/or modify it under the terms of the
 * GNU Lesser General Public License as published by the Free Software Foundation, either version 3
 * of the License, or any later version.
 *
 * DWMBlurGlass is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/
#include "Helper.h"
#include "VersionHelper.h"
#include <io.h>
#include <taskschd.h>
#include <comutil.h>
#include <winrt/base.h>
#include <Shlwapi.h>
#include <Psapi.h>
#include <TlHelp32.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "taskschd.lib")

namespace MDWMBlurGlass
{
	using namespace winrt;

	auto constexpr g_extTaskName = L"DWMBlurGlass_Extend";

	void EnumFiles(std::wstring_view path, std::wstring_view append, std::vector<std::wstring>& fileList)
	{
		_wfinddata_t fileinfo;
		std::wstring p = path.data();
		p += L"\\";
		p += append;
		const intptr_t hFile = _wfindfirst(p.c_str(), &fileinfo);
		if (hFile == -1) return;

		do
		{
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				std::wstring path_ = path.data();
				path_ += L"\\";
				path_ += fileinfo.name;
				fileList.push_back(path_);
			}
		} while (_wfindnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

	std::wstring ReadFileSting(std::wstring_view filePath)
	{
		FILE* file = nullptr;
		_wfopen_s(&file, filePath.data(), L"rb");
		if (!file) return {};

		if (fseek(file, 0L, SEEK_END) != 0) return {};

		const long len = ftell(file);
		size_t read = 0;
		//跳过Unicode文件头
		if (fseek(file, 2, SEEK_SET) != 0) return {};

		std::wstring str;
		for(;;)
		{
			wchar_t wch;
			if (size_t len_read = fread(&wch, 1, sizeof(wchar_t), file))
			{
				read += len_read;
				str += wch;
			}
			if (read + 2 >= (size_t)len)
				break;
		}
		fclose(file);

		return str;
	}

	std::pair<std::wstring, std::wstring> GetSystemLocaleAndParent()
	{
		WCHAR langName[LOCALE_NAME_MAX_LENGTH];
		if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, langName, LOCALE_NAME_MAX_LENGTH))
		{
			std::wstring locale = langName;
			auto pos = locale.find(L'-');
			if (pos != std::wstring::npos)
			{
				return { locale, std::wstring(locale.substr(0, pos)) };
			}
			return { locale, locale };
		}
		return { L"en-US", L"en" };
	}

	std::wstring hrToStr(HRESULT hr)
	{
		WCHAR test[20];
		wsprintfW(test, L"%X", hr);
		std::wstring ret = test;
		ret = L"0x" + ret;
		return ret;
	};

	HMODULE GetModuleBaseAddress(HANDLE hProcess, std::wstring_view dllName)
	{
		DWORD dwNeeded = 0;
		HMODULE hModule = nullptr;

		if (!EnumProcessModules(hProcess, nullptr, 0, &dwNeeded))
		{
			return nullptr;
		}

		DWORD dwModuleCount = dwNeeded / sizeof(HMODULE);
		HMODULE* hModuleList = new(std::nothrow) HMODULE[dwModuleCount];
		if (!hModuleList)
		{
			return nullptr;
		}

		if (!EnumProcessModules(hProcess, hModuleList, dwNeeded, &dwNeeded))
		{
			delete[] hModuleList;
			return nullptr;
		}

		for (DWORD i = 0; i < dwModuleCount; ++i)
		{
			if (const HMODULE hDllModule = hModuleList[i])
			{
				wchar_t pszModuleName[MAX_PATH + 1];
				GetModuleFileNameExW(hProcess, hDllModule, pszModuleName, MAX_PATH);
				if (_wcsicmp(pszModuleName, dllName.data()) == 0)
				{
					hModule = hDllModule;
					break;
				}
			}
		}
		delete[] hModuleList;

		SetLastError(0);
		return hModule;
	}

	bool AdjustPrevileges()
	{
		BOOL bResult = FALSE;
		static const auto& RtlAdjustPrivilege = (DWORD(NTAPI*)(int, BOOL, BOOL, PBOOL))GetProcAddress(GetModuleHandleW(L"Ntdll"), "RtlAdjustPrivilege");
		if (!RtlAdjustPrivilege)
		{
			return false;
		}

		DWORD NtStatus = RtlAdjustPrivilege(20, TRUE, FALSE, &bResult);
		SetLastError(NtStatus);
		return bResult;
	}

	DWORD GetProcessId(std::wstring_view name)
	{
		PROCESSENTRY32W pe;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		pe.dwSize = sizeof(PROCESSENTRY32W);
		if (!Process32FirstW(hSnapshot, &pe))
			return 0;

		while (Process32NextW(hSnapshot, &pe) != FALSE)
		{
			if (_wcsicmp(pe.szExeFile, name.data()) == 0)
			{
				if (name == L"dwmblurglass.exe" && pe.th32ProcessID == GetCurrentProcessId())
					continue;
				return pe.th32ProcessID;
			}
		}
		CloseHandle(hSnapshot);
		return 0;
	}

	bool Inject(DWORD processId, std::wstring_view dllName, std::wstring& err)
	{
		if (!PathFileExistsW(dllName.data()))
		{
			err = L"File not found.";
			return false;
		}

		AdjustPrevileges();

		static const auto& hModule = GetModuleHandleW(L"Kernel32");
		if (!hModule)
		{
			return false;
		}

		HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, processId);
		if (!hProcess)
		{
			err = L"OpenProcess failed!";
			return false;
		}

		LPVOID lpRemoteAddress = nullptr;

		auto clean = RAIIHelper::scope_exit([&]
		{
			if (lpRemoteAddress)
				VirtualFreeEx(hProcess, lpRemoteAddress, 0, MEM_RELEASE);
			CloseHandle(hProcess);
		});

		//目标已存在
		if (GetModuleBaseAddress(hProcess, dllName.data()))
		{
			return true;
		}

		auto size = dllName.length() * sizeof(wchar_t);
		lpRemoteAddress = VirtualAllocEx(hProcess, nullptr, size, MEM_COMMIT, PAGE_READWRITE);
		if (!lpRemoteAddress)
		{
			err = L"VirtualAllocEx failed!";
			return false;
		}

		LPTHREAD_START_ROUTINE lpThreadFun = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryW");
		if (!lpThreadFun)
		{
			err = L"GetProcAddress LoadLibraryW failed!";
			return false;
		}

		if (!WriteProcessMemory(hProcess, lpRemoteAddress, dllName.data(), size, nullptr))
		{
			err = L"WriteProcessMemory failed!";
			return false;
		}

		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, lpThreadFun, lpRemoteAddress, 0, NULL);

		if (!hThread)
		{
			err = L"CreateRemoteThread failed!";
			return false;
		}

		WaitForSingleObject(hThread, INFINITE);

		DWORD dwThreadExitCode = 0;
		if (!GetExitCodeThread(hThread, &dwThreadExitCode))
		{
			err = L"GetExitCodeThread failed!";
			CloseHandle(hThread);
			return false;
		}

		if (!GetModuleBaseAddress(hProcess, dllName.data()))
		{
			err = L"GetModuleBaseAddress did not find the target!";
		}
		CloseHandle(hThread);

		return true;
	}

	bool UnInject(DWORD processId, std::wstring_view dllName, std::wstring& err)
	{
		if (!PathFileExistsW(dllName.data()))
		{
			err = L"File not found.";
			return false;
		}

		AdjustPrevileges();

		static const auto& hModule = GetModuleHandleW(L"Kernel32");
		if (!hModule)
		{
			return false;
		}

		HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, processId);
		if (!hProcess)
		{
			err = L"OpenProcess failed!";
			return false;
		}

		HANDLE hThread = nullptr;

		auto clean = RAIIHelper::scope_exit([&]
		{
			CloseHandle(hProcess);
			if (hThread)
				CloseHandle(hThread);
		});

		LPTHREAD_START_ROUTINE lpThreadFun = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "FreeLibrary");
		if (!lpThreadFun)
		{
			err = L"GetProcAddress FreeLibrary failed!";
			return false;
		}

		LPVOID lpRemoteAddress = GetModuleBaseAddress(hProcess, dllName.data());

		if (!lpRemoteAddress)
		{
			err = L"GetModuleBaseAddress did not find the target!";
			return false;
		}

		hThread = CreateRemoteThread(hProcess, nullptr, 0, lpThreadFun, lpRemoteAddress, 0, nullptr);

		if (!hThread)
		{
			err = L"CreateRemoteThread failed!";
			return false;
		}

		WaitForSingleObject(hThread, INFINITE);

		DWORD dwThreadExitCode = 0;
		if (!GetExitCodeThread(hThread, &dwThreadExitCode))
		{
			err = L"GetExitCodeThread failed!";
			return false;
		}

		if (GetModuleBaseAddress(hProcess, dllName.data()))
		{
			err = L"Reference count is not 0";
			return false;
		}

		return true;
	}

	bool ModuleIsExists(DWORD processId, std::wstring_view dllName)
	{
		const HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, processId);
		if (!hProcess)
			return false;

		if (GetModuleBaseAddress(hProcess, dllName.data()))
		{
			CloseHandle(hProcess);
			return true;
		}
		CloseHandle(hProcess);
		return false;
	}

	bool IsRunasAdmin()
	{
		bool bElevated = false;
		HANDLE hToken = nullptr;

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			return FALSE;

		TOKEN_ELEVATION tokenEle;
		DWORD dwRetLen = 0;

		if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen))
		{
			if (dwRetLen == sizeof(tokenEle))
			{
				bElevated = tokenEle.TokenIsElevated > 0;
			}
		}

		CloseHandle(hToken);
		return bElevated;
	}

	std::wstring ReplaceString(std::wstring str, std::wstring_view old_value, std::wstring_view new_value)
	{
		if (old_value.empty() || str.empty())
			return str;
		size_t cur = 0;
		while (true)
		{
			if (std::wstring::size_type pos; (pos = str.find(old_value, cur)) != std::wstring::npos)
			{
				str.replace(pos, old_value.length(), new_value);
				cur = pos + 1;
			}
			else
				break;
		}
		return str;
	}
}
