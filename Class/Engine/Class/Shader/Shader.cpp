#include "Shader.h"

// デストラクタ
Shader::~Shader()
{

 }

// 初期化
void Shader::Initialize()
{
	/*--------------------
	    DXCを初期化する
	--------------------*/

	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

// シェーダーをコンパイルする
IDxcBlob* Shader::CompilerShader(std::ostream& os,const std::wstring& filePath, const wchar_t* profile)
{
	// コンパイルしますよというログ
	Log(os,ConvertString(std::format(L"Begin CompileShader, path : {}, profile : {}", filePath, profile)));

	/*-----------------------
	    HLSLファイルを読む
	-----------------------*/

	// HLSLファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;


	/*-----------------
	    Compileする
	-----------------*/

	LPCWSTR arguments[] =
	{
		// コンパイル対象のhlslファイル名
		filePath.c_str(),

		// エントリーポイントの指定
		L"-E" , L"main",

		// ShaderProfileの設定
		L"-T" , profile ,

		// デバッグ用の情報を埋め込む
		L"-Zi" , L"-Qembed_debug",

		// 最適化を外しておく
		L"-Od",

		// メモリレイアウトは行優先
		L"-Zpr"
	};

	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile
	(
		// 読み込んだファイル
		&shaderSourceBuffer,

		// コンパイルオプション
		arguments,

		// コンパイルオプションの数
		_countof(arguments),

		// includeが含まれた諸々
		includeHandler_,

		// コンパイル結果
		IID_PPV_ARGS(&shaderResult)
	);

	assert(SUCCEEDED(hr));


	/*----------------------------------
	    警告・エラーが出ていないか確認する
	----------------------------------*/

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Log(os,shaderError->GetStringPointer());

		assert(false);
	}


	/*--------------------------------
	    Compiler結果を受け取って返す
	--------------------------------*/

	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// 成功したよというログ
	Log(os,ConvertString(std::format(L"Compile Succeeded, path : {} , profile : {} \n" , filePath , profile)));

	// もう使わないリソースを解放する
	shaderSource->Release();
	shaderResult->Release();

	return shaderBlob;
}