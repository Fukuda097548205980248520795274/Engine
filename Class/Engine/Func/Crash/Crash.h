#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <filesystem>
#include <strsafe.h>
#include <fstream>
#include <chrono>
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception)
{
	// 時刻を取得して、時刻を名前に入れたファイルを作成
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"Class/Engine/Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"Class/Engine/Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	// ProcessId（このexeのId）とクラッシュ（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION mindumpInformation{ 0 };
	mindumpInformation.ThreadId = threadId;
	mindumpInformation.ExceptionPointers = exception;
	mindumpInformation.ClientPointers = TRUE;

	// Dumpを出力する
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &mindumpInformation, nullptr, nullptr);

	return EXCEPTION_EXECUTE_HANDLER;
}