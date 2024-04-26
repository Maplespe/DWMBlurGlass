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
	using namespace Mui;
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

		const _m_long len = ftell(file);
		_m_size read = 0;
		//跳过Unicode文件头
		if (fseek(file, 2, SEEK_SET) != 0) return {};

		std::wstring str;
		for(;;)
		{
			wchar_t wch;
			if (_m_size len_read = fread(&wch, 1, sizeof(wchar_t), file))
			{
				read += len_read;
				str += wch;
			}
			if (read + 2 >= (_m_size)len)
				break;
		}
		fclose(file);

		return str;
	}

	std::wstring GetSystemLocalName()
	{
		WCHAR langName[LOCALE_NAME_MAX_LENGTH];
		if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, langName, LOCALE_NAME_MAX_LENGTH)) 
		{
			return langName;
		}
		return L"en-US";
	}

	BOOL EnableHostBackdropBrush(HWND hWnd)
	{
		struct WINDOWCOMPOSITIONATTRIBUTEDATA
		{
			DWORD dwAttribute;
			PVOID pvData;
			SIZE_T cbData;
		};
		struct ACCENT_POLICY
		{
			DWORD AccentState;
			DWORD AccentFlags;
			DWORD dwGradientColor;
			DWORD dwAnimationId;
		};

		ACCENT_POLICY policy = {};

		if (os::buildNumber < 17763)
			policy.AccentState = 3;
		else
			policy.AccentState = 5;

		WINDOWCOMPOSITIONATTRIBUTEDATA data =
		{
			19,
			&policy,
			sizeof(policy)
		};
		static auto SetWindowCompositionAttribute = reinterpret_cast<BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBUTEDATA*)>(
			GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));

		return SetWindowCompositionAttribute(hWnd, &data);
	}

	std::wstring hrToStr(HRESULT hr)
	{
		WCHAR test[20];
		wsprintfW(test, L"%X", hr);
		std::wstring ret = test;
		ret = L"0x" + ret;
		return ret;
	};

	bool InstallScheduledTasks(std::wstring& errinfo)
	{
		//创建任务服务对象
		com_ptr<ITaskService> taskService = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, taskService.put_void());
		if (FAILED(hr))
		{
			errinfo = L"CoCreateInstance failed! errcode: " + hrToStr(hr);
			return false;
		}

		//连接到任务服务
		hr = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
		if (FAILED(hr))
		{
			errinfo = L"taskService->Connect failed! errcode: " + hrToStr(hr);
			return false;
		}

		//获取根任务文件夹
		com_ptr<ITaskFolder> rootFolder = nullptr;
		hr = taskService->GetFolder(_bstr_t("\\"), rootFolder.put());
		if (FAILED(hr))
		{
			errinfo = L"taskService->GetFolder failed! errcode: " + hrToStr(hr);
			return false;
		}

		//创建任务定义对象来创建任务
		com_ptr<ITaskDefinition> taskDefinition = nullptr;
		hr = taskService->NewTask(0, taskDefinition.put());
		if (FAILED(hr))
		{
			errinfo = L"taskService->NewTask failed! errcode: " + hrToStr(hr);
			return false;
		}

		//设置注册信息
		com_ptr<IRegistrationInfo> regInfo = nullptr;
		hr = taskDefinition->get_RegistrationInfo(regInfo.put());
		if (FAILED(hr))
		{
			errinfo = L"taskDefinition->get_RegistrationInfo failed! errcode: " + hrToStr(hr);
			return false;
		}

		//设置作者信息
		regInfo->put_Author((BSTR)L"DWMBlurGlass");

		//设置登录类型和运行权限
		com_ptr<IPrincipal> principal = nullptr;
		hr = taskDefinition->get_Principal(principal.put());
		if (FAILED(hr))
		{
			errinfo = L"taskDefinition->get_Principal failed! errcode: " + hrToStr(hr);
			return false;
		}
		//设置登录类型
		principal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);

		//设置运行权限
		//管理员权限
		principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
		//设置其他信息
		com_ptr<ITaskSettings> setting = nullptr;
		hr = taskDefinition->get_Settings(setting.put());
		if (FAILED(hr))
		{
			errinfo = L"taskDefinition->get_Settings failed! errcode: " + hrToStr(hr);
			return false;
		}
		//设置其他信息
		setting->put_StopIfGoingOnBatteries(VARIANT_FALSE);
		setting->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
		setting->put_AllowDemandStart(VARIANT_TRUE);
		setting->put_StartWhenAvailable(VARIANT_FALSE);
		setting->put_MultipleInstances(TASK_INSTANCES_PARALLEL);

		//创建执行动作
		com_ptr<IActionCollection> actioncoll = nullptr;
		hr = taskDefinition->get_Actions(actioncoll.put());
		if (FAILED(hr))
		{
			errinfo = L"taskDefinition->get_Actions failed! errcode: " + hrToStr(hr);
			return false;
		}
		com_ptr<IAction> action = nullptr;
		//创建执行操作
		actioncoll->Create(TASK_ACTION_EXEC, action.put());

		com_ptr<IExecAction> pExecAction = nullptr;
		hr = action->QueryInterface(IID_IExecAction, pExecAction.put_void());
		if (FAILED(hr))
		{
			errinfo = L"action->QueryInterface failed! errcode: " + hrToStr(hr);
			return false;
		}
		
		//设置程序路径和参数
		pExecAction->put_Path((BSTR)(Utils::GetCurrentDir() + L"\\DWMBlurGlass.exe").c_str());
		pExecAction->put_Arguments((BSTR)L"runhost");
		
		//设置触发器信息，包括用户登录时触发
		com_ptr<ITriggerCollection> triggercoll = nullptr;
		hr = taskDefinition->get_Triggers(triggercoll.put());
		if (FAILED(hr))
		{
			errinfo = L"taskDefinition->get_Triggers failed! errcode: " + hrToStr(hr);
			return false;
		}

		//创建触发器
		com_ptr<ITrigger> trigger = nullptr;
		hr = triggercoll->Create(TASK_TRIGGER_LOGON, trigger.put());
		if (FAILED(hr))
		{
			errinfo = L"triggercoll->Create failed! errcode: " + hrToStr(hr);
			return false;
		}

		//注册任务计划
		com_ptr<IRegisteredTask> registeredTask = nullptr;
		hr = rootFolder->RegisterTaskDefinition((BSTR)g_extTaskName,
			taskDefinition.get(),
			TASK_CREATE_OR_UPDATE,
			_variant_t(),
			_variant_t(),
			TASK_LOGON_INTERACTIVE_TOKEN,
			_variant_t(""),
			registeredTask.put());

		if (FAILED(hr))
			errinfo = L"rootFolder->RegisterTaskDefinition failed! errcode: " + hrToStr(hr);

		return SUCCEEDED(hr);
	}

	bool IsInstallTasks()
	{
		com_ptr<ITaskService> taskService = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, taskService.put_void());
		if (FAILED(hr))
			return false;

		hr = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
		if (FAILED(hr))
			return false;

		com_ptr<ITaskFolder> rootFolder = nullptr;
		hr = taskService->GetFolder(_bstr_t("\\"), rootFolder.put());
		if (FAILED(hr))
			return false;

		com_ptr<IRegisteredTask> task = nullptr;
		hr = rootFolder->GetTask((BSTR)g_extTaskName, task.put());

		return SUCCEEDED(hr);
	}

	bool DeleteScheduledTasks(std::wstring& errinfo)
	{
		com_ptr<ITaskService> taskService = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService, taskService.put_void());
		if (FAILED(hr))
		{
			errinfo = L"CoCreateInstance failed! errcode: " + hrToStr(hr);
			return false;
		}

		hr = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
		if (FAILED(hr))
		{
			errinfo = L"taskService->Connect failed! errcode: " + hrToStr(hr);
			return false;
		}

		com_ptr<ITaskFolder> rootFolder = nullptr;
		hr = taskService->GetFolder(_bstr_t("\\"), rootFolder.put());
		if (FAILED(hr))
		{
			errinfo = L"taskService->GetFolder failed! errcode: " + hrToStr(hr);
			return false;
		}

		hr = rootFolder->DeleteTask((BSTR)g_extTaskName, 0);
		return SUCCEEDED(hr);
	}

	bool BrowseForFile(bool isOpen, bool multiple, const std::vector<COMDLG_FILTERSPEC>& filter, HWND parentWnd, std::vector<std::wstring>& selectedFiles, std::wstring_view defExtName)
	{
		IFileDialog* fileDialog = nullptr;
		DWORD flags = FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST;
		if (multiple)
			flags |= FOS_ALLOWMULTISELECT;

		HRESULT hr = CoCreateInstance(isOpen ? CLSID_FileOpenDialog : CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER,
			isOpen ? IID_IFileOpenDialog : IID_IFileSaveDialog, (LPVOID*)&fileDialog);
		if (FAILED(hr))
			return false;

		//设置文件过滤器
		fileDialog->SetFileTypes((UINT)filter.size(), filter.data());

		//设置Flag
		FILEOPENDIALOGOPTIONS options;
		fileDialog->GetOptions(&options);
		fileDialog->SetOptions(options | flags);
		fileDialog->SetDefaultExtension(defExtName.data());
		hr = fileDialog->Show(parentWnd);
		if (FAILED(hr))
		{
			fileDialog->Release();
			return false;
		}
		if (multiple && isOpen)
		{
			IShellItemArray* pItems = nullptr;
			static_cast<IFileOpenDialog*>(fileDialog)->GetResults(&pItems);

			if (!pItems)
			{
				fileDialog->Release();
				return false;
			}

			DWORD itemCount;
			pItems->GetCount(&itemCount);
			selectedFiles.reserve(itemCount);
			for (DWORD i = 0; i < itemCount; ++i)
			{
				IShellItem* pItem = nullptr;
				pItems->GetItemAt(i, &pItem);
				if (!pItem)
					continue;

				PWSTR pszFilePath;
				pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				selectedFiles.emplace_back(pszFilePath);
				CoTaskMemFree(pszFilePath);
				pItem->Release();
			}
			pItems->Release();
		}
		else
		{
			IShellItem* pItems = nullptr;
			fileDialog->GetResult(&pItems);
			if (!pItems)
			{
				fileDialog->Release();
				return false;
			}
			PWSTR pszFilePath;
			pItems->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
			selectedFiles.emplace_back(pszFilePath);
			CoTaskMemFree(pszFilePath);
			pItems->Release();
		}
		fileDialog->Release();
		return true;
	}

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
}