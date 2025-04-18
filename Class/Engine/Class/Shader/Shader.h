#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include "../../Func/StringInfo/StringInfo.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")


class Shader
{
public:

	// デストラクタ
	~Shader();

	// 初期化
	void Initialize();

	// シェーダーをコンパイルする
	IDxcBlob* CompilerShader(std::ostream& os, const std::wstring& filePath, const wchar_t* profile);


private:

	// dxcUtils
	IDxcUtils* dxcUtils_ = nullptr;

	// dxcCompiler
	IDxcCompiler3* dxcCompiler_ = nullptr;

	// includeHandle
	IDxcIncludeHandler* includeHandler_ = nullptr;

};

