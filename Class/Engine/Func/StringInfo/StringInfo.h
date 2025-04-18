#pragma once
#include <Windows.h>
#include <string>
#include <fstream>

// string -> wstring に変換する
std::wstring ConvertString(const std::string& str);

// wstring -> string に変換する
std::string ConvertString(const std::wstring& str);

/// <summary>
/// ログを表示する
/// </summary>
/// <param name="message">文字列</param>
void Log(std::ostream& os,const std::string& message);