#include "Engine.h"

// デストラクタ
Engine::~Engine()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	pixelShaderBlob_->Release();
	vertexShaderBlob_->Release();
	rootSignature_->Release();
	if (errorBlob_)
	{
		errorBlob_->Release();
	}
	signatureBlob_->Release();

	// シェーダー
	delete shader_;

	// サウンド
	delete sound_;

	// モデルマネージャ
	delete modelManager_;

	// テクスチャマネージャ
	delete textureManager_;

	// フェンス
	delete fence_;

	// スワップチェーン
	delete swapChain_;
	
	// コマンドリスト
	delete commands_;

	// エラー検知
	delete errorDetection_;

	// 入力
	delete input_;

	// ウィンドウ
	delete window_;

	// COMの終了
	CoUninitialize();
}

// 初期化
void Engine::Initialize(const int32_t kClientWidth , const int32_t kClientHeight)
{
	/*-------------------------
	    ログファイルを書き出す
	-------------------------*/

	// ログのディレクトリを用意する
	std::filesystem::create_directory("Class/Engine/Logs");

	// 現在時刻を取得
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds =
		std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 日本時間（PCの設定時間）に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m&d_%H%M%S}", localTime);

	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("Class/Engine/Logs/") + dateString + ".log";

	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);



	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// miniDump
	SetUnhandledExceptionFilter(ExportDump);


	// ウィンドウの生成と初期化
	window_ = new Window();
	window_->Initialize(kClientWidth,kClientHeight);

	// 入力の生成と初期化
	input_ = new Input();
	input_->Initialize(window_->GetWndClass().hInstance , window_->GetHwnd());

	// エラーを感知するクラス
	errorDetection_ = new ErrorDetection();
	errorDetection_->Initialize();


	/*-----------------------
	    DirectXを初期化する
	-----------------------*/

	// DXGIfactoryを取得する
	dxgiFactory_ = GetDXGIFactory();

	// 使用するアダプタ（GPU）を取得する
	useAdapter_ = GetUseAdapter(logStream,dxgiFactory_);

	// Deviceを取得する
	device_ = GetDevice(logStream,useAdapter_);

	// 初期化完了!!!
	Log(logStream,"Complate create ID3D12Device!! \n");


	// エラーを検知したら停止する
	errorDetection_->MakeItStop(device_);


	// コマンドの生成と初期化
	commands_ = new Commands();
	commands_->Initialize(device_);


	/*----------------------------
	    DescriptorHeapを生成する
	----------------------------*/

	// RTV用のディスクリプタヒープ
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kNumRtvDescriptor_, false);

	// SRV用のディスクリプタヒープ
	srvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kNunSrvDescriptor_, true);

	// DSV用のディスクリプタヒープ
	dsvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, kNunDsvDescriptor_, false);


	/*------------------------
	    wapChainを生成する
	------------------------*/

	// swapChainの生成と初期化
	swapChain_ = new SwapChain();
	swapChain_->Initialize(window_->GetHwnd(), dxgiFactory_, device_, commands_->GetCommandQueue(), kClientWidth, kClientHeight);


	// SwapChainからResourceを引っ張ってくる
	HRESULT hr = swapChain_->GetSwapChain()->GetBuffer(0 , IID_PPV_ARGS(&swapChainResource_[0]));
	assert(SUCCEEDED(hr));
	
	hr = swapChain_->GetSwapChain()->GetBuffer(1, IID_PPV_ARGS(&swapChainResource_[1]));
	assert(SUCCEEDED(hr));

	
	/*---------------
	    RTVを作る
	---------------*/

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	// 出力結果をSRGBに変換して書き込む
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 2Dテクスチャ
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


	// 1つめ　先頭
	rtvHandles_[0] = GetCPUDescriptorHandle(rtvDescriptorHeap_, device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV), 0);
	device_->CreateRenderTargetView(swapChainResource_[0].Get(), &rtvDesc, rtvHandles_[0]);

	// 2つめ
	rtvHandles_[1] = GetCPUDescriptorHandle(rtvDescriptorHeap_, device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV), 1);
	device_->CreateRenderTargetView(swapChainResource_[1].Get(), &rtvDesc, rtvHandles_[1]);


	// Fenceの生成と初期化
	fence_ = new Fence();
	fence_->Initialize(device_);


	
	// テクスチャマネージャの初期化と生成
	textureManager_ = new TextureManager();
	textureManager_->Initialize();

	// モデルマネージャの初期化と生成
	modelManager_ = new ModelManager();
	modelManager_->Initialize();

	// サウンドの初期化と生成
	sound_ = new Sound();
	sound_->Initialize();



	/*------------------
	    DSVを設定する
	------------------*/

	depthStencilResource_ = CreateDepthStencilTextureResource(device_, kClientWidth, kClientHeight);

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// DSVheapの先頭に配置する
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());


	/*-------------
	    描画設定
	-------------*/

	// シェーダーの初期化と生成
	shader_ = new Shader();
	shader_->Initialize();


	// ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	/*   RootSignature   */

	// ルートパラメータ
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// PixelShader CBV 0
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// VertexShader CBV 0
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// PixelShader DescriptorTable
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	// PixelShader CBV 1
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;


	// サンプラーを設定する
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NONE;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;

	// PixelShader s0
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	// ルートシグネチャを設定する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	// シリアライズにしてバイナリにする
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr))
	{
		Log(logStream,reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	hr = device_->CreateRootSignature(0, 
		signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));



	/*   InputLayout   */

	// 頂点シェーダのどの変数にinputするかを選ぶ
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};

	// float4 position : POSITION0;
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// float2 texcoord : TEXCOORD0
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// float3 normal : NORMAL0
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// 頂点シェーダにinputするデータ
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescs{};
	inputLayoutDescs.pInputElementDescs = inputElementDescs;
	inputLayoutDescs.NumElements = _countof(inputElementDescs);



	/*   BlendState   */

	/*

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	*/

	// ピクセルシェーダの出力の設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	/*   RasterizeState   */

	// ラスタライザの設定
	D3D12_RASTERIZER_DESC rasterizeDesc{};

	// 裏面（時計回り）を表示しない
	rasterizeDesc.CullMode = D3D12_CULL_MODE_BACK;

	// 三角形を塗りつぶす
	rasterizeDesc.FillMode = D3D12_FILL_MODE_SOLID;


	/*   シェーダーをコンパイルする   */

	vertexShaderBlob_ = shader_->CompilerShader(logStream,L"./Class/Engine/Shader/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = shader_->CompilerShader(logStream,L"./Class/Engine/Shader/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob_ != nullptr);


	/*   DepthStencilState   */

	// デプスステンシルの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// Depthの機能を有効にする
	depthStencilDesc.DepthEnable = true;

	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	// 比較関数はLessEqual 近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/*   全ての描画設定を詰め込む   */

	// PSOの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_;
	graphicsPipelineStateDesc.InputLayout = inputLayoutDescs;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer() , vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer() , pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizeDesc;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 書き込むDSVの情報
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 利用するトポロジ（形状）のタイプ
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 設定を基に生成する
	hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));


	/*   ビューポートとシザー   */

	viewport_.Width = static_cast<float>(kClientWidth);
	viewport_.Height = static_cast<float>(kClientHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	scissorRect_.left = 0;
	scissorRect_.right = kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = kClientHeight;


	/*----------------------
	    ImGuiを初期化する
	----------------------*/

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window_->GetHwnd());
	ImGui_ImplDX12_Init(device_.Get(), swapChain_->GetSwapChainDesc().BufferCount, rtvDesc.Format,
		srvDescriptorHeap_.Get(),
		srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}

// ウィンドウが開いているかどうか
bool Engine::IsWindowOpen()
{
	if (window_->ProcessMessage())
	{
		return true;
	}

	return false;
}

// フレーム開始
void Engine::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	input_->Acquire();

	// バックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// バックバッファの状態を Present -> RenderTarget に遷移させる
	TransitionBarrier(commands_->GetCommandList(), swapChainResource_[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	commands_->GetCommandList()->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandle);

	// 指定した色で画面をクリアする
	float clearColor[] = { 0.1f , 0.25f , 0.5f , 1.0f };
	commands_->GetCommandList()->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
	commands_->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0,nullptr);

	// 描画用のDescriptorの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap_.Get()};
	commands_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

// フレーム終了
void Engine::EndFrame()
{

	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commands_->GetCommandList().Get());

	// バックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// バックバッファの状態を RenderTarget -> Present に遷移させる
	TransitionBarrier(commands_->GetCommandList(), swapChainResource_[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// コマンドリストの内容を確定させる
	HRESULT hr = commands_->GetCommandList()->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commands_->GetCommandList().Get()};
	commands_->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// GPUとOSに画面の交換を行うように通知する
	swapChain_->GetSwapChain()->Present(1, 0);

	// GPUの完了を待つ
	fence_->WaitForGPU(commands_->GetCommandQueue());

	// 次のフレーム用のコマンドリストを準備
	hr = commands_->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = commands_->GetCommandList()->Reset(commands_->GetCommandAllocator().Get(), nullptr);
	assert(SUCCEEDED(hr));


	// 使用したリソースのアドレスを消す
	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i])
		{
			resourceMemories[i] = nullptr;
		}
	}

	input_->CopyKeys();
}

// テクスチャを読み込む
uint32_t Engine::LoadTexture(const std::string& filePath)
{
	return textureManager_->LoadTextureGetNumber(filePath, device_, srvDescriptorHeap_, commands_->GetCommandList());
}

// モデルデータを読み込む
uint32_t Engine::LoadModelData(const std::string& directory, const std::string& fileName)
{
	uint32_t modelNumber = modelManager_->LoadModelGetNumber(directory, fileName);
	modelManager_->SetTextureNumber(modelNumber,
		textureManager_->LoadTextureGetNumber(modelManager_->GetModelData(modelNumber).material.textureFilePath,
			device_, srvDescriptorHeap_, commands_->GetCommandList()));

	return modelNumber;
}

// サウンドデータを読み込む
uint32_t Engine::LoadSound(const char* fileName)
{
	return sound_->LoadSoundGetNumber(fileName);
}

// サウンドデータを再生する
void Engine::PlayerSoundWav(uint32_t soundHandle)
{
	sound_->SelectNumberPlaySoundWav(soundHandle);
}

// キー操作（Press）
UINT Engine::PushPressKeys(BYTE key)
{
	return input_->PressKeys(key);
}

// キー操作（Trigger）
UINT Engine::PushTriggerKeys(BYTE key)
{
	return input_->TriggerKeys(key);
}

// キー操作（Release）
UINT Engine::PushReleaseKeys(BYTE key)
{
	return input_->ReleaseKeys(key);
}

// 三角形を描画する
void Engine::DrawTriangle(struct Transform3D& transform,const Matrix4x4& viewProjectionMatrix, uint32_t textureHandle, Vector3 color)
{
	// ビューポートの設定
	commands_->GetCommandList()->RSSetViewports(1, &viewport_);
	
	// シザーの設定
	commands_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);

	// rootSignature
	commands_->GetCommandList()->SetGraphicsRootSignature(rootSignature_);

	// PSOの設定
	commands_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());


	// 頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device_, sizeof(VertexData) * 6);

	// VBVを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// データを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { 0.0f , 0.5f , 0.0f , 1.0f };
	vertexData[0].texcoord = { 0.5f , 0.0f };
	vertexData[0].normal = { 0.0f,0.0f,0.0f };
	vertexData[1].position = { 0.5f , -0.5f , 0.0f , 1.0f };
	vertexData[1].texcoord = { 1.0f , 1.0f };
	vertexData[1].normal = { 0.0f,0.0f,0.0f };
	vertexData[2].position= { -0.5f , -0.5f , 0.0f , 1.0f };
	vertexData[2].texcoord = { 0.0f , 1.0f };
	vertexData[2].normal = { 0.0f,0.0f,0.0f };
	vertexData[3].position = { 0.0f , 0.0f , 0.0f , 1.0f };
	vertexData[3].texcoord = { 0.5f , 0.0f };
	vertexData[3].normal = { 0.0f,0.0f,0.0f };
	vertexData[4].position = { 0.0f , -0.5f , -0.5f , 1.0f };
	vertexData[4].texcoord = { 1.0f , 1.0f };
	vertexData[4].normal = { 0.0f,0.0f,0.0f };
	vertexData[5].position = { 0.0f , -0.5f , 0.5f , 1.0f };
	vertexData[5].texcoord = { 0.0f , 1.0f };
	vertexData[5].normal = { 0.0f,0.0f,0.0f };


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device_, sizeof(Material));

	// マテリアルに書き込むデータ
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	Transform3D uvTransform = { {1.0f , 1.0f , 1.0f} , {0.0f , 0.0f , 0.0f} , {0.0f , 0.0f , 0.0f} };

	materialData->color = {color.x , color.y , color.z , 1.0f};
	materialData->enableLighting = false;
	materialData->uvTransform = Multiply(Multiply(Make4x4ScaleMatrix(uvTransform.scale),
		Make4x4RotateZMatrix(uvTransform.rotate.z)), Make4x4TranslateMatrix(uvTransform.translate));


	// 座標変換用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = CreateBufferResource(device_, sizeof(TransformationMatrix));

	// 行列に書き込む
	TransformationMatrix* transformationMatrixData = nullptr;
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->world = Make4x4AffineMatrix(transform.scale, transform.rotate, transform.translate);
	transformationMatrixData->worldViewProjection = Multiply(transformationMatrixData->world, viewProjectionMatrix);


	// VBVを設定する
	commands_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 形状を設定
	commands_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標変換用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// テクスチャのCBVを設定する
	textureManager_->SelectTexture(textureHandle, commands_->GetCommandList());

	// 描画する
	commands_->GetCommandList()->DrawInstanced(6, 1, 0, 0);

	
	/*--------------------------------------
	    使用していないリソースメモリに記録する
	--------------------------------------*/

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = vertexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = materialResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = transformationMatrixResource;
			break;
		}
	}
}


// スプライトを描画する
void Engine::DrawSprite(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	const Transform3D& transform, const Matrix4x4& viewOrthograhpicsMatrix, uint32_t textureHandle)
{
	// ビューポートの設定
	commands_->GetCommandList()->RSSetViewports(1, &viewport_);

	// シザーの設定
	commands_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);

	// rootSignature
	commands_->GetCommandList()->SetGraphicsRootSignature(rootSignature_);

	// PSOの設定
	commands_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());


	// インデックスリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = CreateBufferResource(device_, sizeof(uint32_t) * 6);

	// IBVを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// データを書き込む
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;



	// 頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device_, sizeof(VertexData) * 4);

	// VBVを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// データを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { x3 , y3 , 0.0f , 1.0f };
	vertexData[0].texcoord = { 0.0f , 1.0f };
	vertexData[0].normal = { 0.0f , 0.0f , -1.0f };

	vertexData[1].position = { x1 , y1 , 0.0f , 1.0f };
	vertexData[1].texcoord = { 0.0f , 0.0f };
	vertexData[1].normal = { 0.0f , 0.0f , -1.0f };

	vertexData[2].position = { x4 , y4 , 0.0f , 1.0f };
	vertexData[2].texcoord = { 1.0f , 1.0f };
	vertexData[2].normal = { 0.0f , 0.0f , -1.0f };

	vertexData[3].position = { x2 , y2 , 0.0f , 1.0f };
	vertexData[3].texcoord = { 1.0f , 0.0f };
	vertexData[3].normal = { 0.0f , 0.0f , -1.0f };


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device_, sizeof(Material));

	// データを書き込む
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	Transform3D uvTransform = { {1.0f , 1.0f , 1.0f} , {0.0f , 0.0f , 0.0f} , {0.0f , 0.0f , 0.0f} };

	materialData->color = { 1.0f , 1.0f , 1.0f , 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = Multiply(Multiply(Make4x4ScaleMatrix(uvTransform.scale),
		Make4x4RotateZMatrix(uvTransform.rotate.z)), Make4x4TranslateMatrix(uvTransform.translate));


	// 座標変換用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = CreateBufferResource(device_, sizeof(TransformationMatrix));

	// 行列に書き込む
	TransformationMatrix* transformationMatrixData = nullptr;
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->world = Make4x4AffineMatrix(transform.scale, transform.rotate, transform.translate);
	transformationMatrixData->worldViewProjection = Multiply(transformationMatrixData->world, viewOrthograhpicsMatrix);


	// IBVを設定する
	commands_->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	// VBVを設定する
	commands_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 形状を設定
	commands_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標変換用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// テクスチャのCBVを設定する
	textureManager_->SelectTexture(textureHandle, commands_->GetCommandList());

	// 描画する
	commands_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);


	/*--------------------------------------
		使用していないリソースメモリに記録する
	--------------------------------------*/

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = indexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = vertexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = materialResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = transformationMatrixResource;
			break;
		}
	}
}

// 球を描画する
void Engine::DrawSphere(uint32_t subdivisions,const Transform3D& transform, const Matrix4x4& viewProjectionMatrix,
	const DirectionalLight& light, uint32_t textureHandle)
{
	// ビューポートの設定
	commands_->GetCommandList()->RSSetViewports(1, &viewport_);

	// シザーの設定
	commands_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);

	// rootSignature
	commands_->GetCommandList()->SetGraphicsRootSignature(rootSignature_);

	// PSOの設定
	commands_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());


	// インデックスリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = CreateBufferResource(device_, sizeof(uint32_t) * (subdivisions * subdivisions * 6));

	// IBVを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * (subdivisions * subdivisions * 6);
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// データを書き込む
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < subdivisions; ++latIndex)
	{
		for (uint32_t lonIndex = 0; lonIndex < subdivisions; ++lonIndex)
		{
			// 要素数
			uint32_t index = (latIndex * subdivisions + lonIndex) * 6;

			// 頂点の要素数
			uint32_t vertexIndex = (latIndex * subdivisions + lonIndex) * 4;

			indexData[index + 0] = vertexIndex + 0;
			indexData[index + 1] = vertexIndex + 1;
			indexData[index + 2] = vertexIndex + 2;
			indexData[index + 3] = vertexIndex + 2;
			indexData[index + 4] = vertexIndex + 1;
			indexData[index + 5] = vertexIndex + 3;
		}
	}


	// 頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device_, sizeof(VertexData) * (subdivisions * subdivisions * 4));

	// VBVを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * (subdivisions * subdivisions * 4);
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// データを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	
	// 経度分割1つ分の角度φ
	const float kLonEvery = float(M_PI) * 2.0f / static_cast<float>(subdivisions);

	// 緯度分割1つ分の角度Θ
	const float kLatEvery = float(M_PI) / static_cast<float>(subdivisions);

	// 緯度の方向に分割
	for (uint32_t latIndex = 0; latIndex < subdivisions; ++latIndex)
	{
		// 現在の緯度
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;

		// 経度の方向に分割
		for (uint32_t lonIndex = 0; lonIndex < subdivisions; ++lonIndex)
		{
			// 現在の経度
			float lon = lonIndex * kLonEvery;

			// 要素数
			uint32_t index = (latIndex * subdivisions + lonIndex) * 4;

			vertexData[index].position.x = std::cos(lat) * std::cos(lon);
			vertexData[index].position.y = std::sin(lat);
			vertexData[index].position.z = std::cos(lat) * std::sin(lon);
			vertexData[index].position.w = 1.0f;
			vertexData[index].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(subdivisions);
			vertexData[index].texcoord.y = 1.0f - static_cast<float>(latIndex) / static_cast<float>(subdivisions);
			vertexData[index].normal.x = vertexData[index].position.x;
			vertexData[index].normal.y = vertexData[index].position.y;
			vertexData[index].normal.z = vertexData[index].position.z;

			vertexData[index + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertexData[index + 1].position.y = std::sin(lat + kLatEvery);
			vertexData[index + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertexData[index + 1].position.w = 1.0f;
			vertexData[index + 1].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(subdivisions);
			vertexData[index + 1].texcoord.y = 1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(subdivisions);
			vertexData[index + 1].normal.x = vertexData[index + 1].position.x;
			vertexData[index + 1].normal.y = vertexData[index + 1].position.y;
			vertexData[index + 1].normal.z = vertexData[index + 1].position.z;

			vertexData[index + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexData[index + 2].position.y = std::sin(lat);
			vertexData[index + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexData[index + 2].position.w = 1.0f;
			vertexData[index + 2].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(subdivisions);
			vertexData[index + 2].texcoord.y = 1.0f - static_cast<float>(latIndex) / static_cast<float>(subdivisions);
			vertexData[index + 2].normal.x = vertexData[index + 2].position.x;
			vertexData[index + 2].normal.y = vertexData[index + 2].position.y;
			vertexData[index + 2].normal.z = vertexData[index + 2].position.z;

			vertexData[index + 3].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
			vertexData[index + 3].position.y = std::sin(lat + kLatEvery);
			vertexData[index + 3].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
			vertexData[index + 3].position.w = 1.0f;
			vertexData[index + 3].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(subdivisions);
			vertexData[index + 3].texcoord.y = 1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(subdivisions);
			vertexData[index + 3].normal.x = vertexData[index + 3].position.x;
			vertexData[index + 3].normal.y = vertexData[index + 3].position.y;
			vertexData[index + 3].normal.z = vertexData[index + 3].position.z;
		}
	}


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device_, sizeof(Material));

	// マテリアルに書き込むデータ
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	Transform3D uvTransform = { {1.0f , 1.0f , 1.0f} , {0.0f , 0.0f , 0.0f} , {0.0f , 0.0f , 0.0f} };

	materialData->color = { 1.0f , 1.0f , 1.0f , 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = Multiply(Multiply(Make4x4ScaleMatrix(uvTransform.scale),
		Make4x4RotateZMatrix(uvTransform.rotate.z)), Make4x4TranslateMatrix(uvTransform.translate));


	// 座標変換用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = CreateBufferResource(device_, sizeof(TransformationMatrix));

	// 行列に書き込む
	TransformationMatrix* transformationMatrixData = nullptr;
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->world = Make4x4AffineMatrix(transform.scale, transform.rotate, transform.translate);
	transformationMatrixData->worldViewProjection = Multiply(transformationMatrixData->world, viewProjectionMatrix);


	// 平行光源用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device_, sizeof(DirectionalLight));

	// データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = light.color;
	directionalLightData->direction = light.direction;
	directionalLightData->intensity = light.intensity;


	// IBVを設定する
	commands_->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	// VBVを設定する
	commands_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 形状を設定
	commands_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標変換用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// 平行光源用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	// テクスチャのCBVを設定する
	textureManager_->SelectTexture(textureHandle, commands_->GetCommandList());

	// 描画する
	commands_->GetCommandList()->DrawIndexedInstanced(subdivisions * subdivisions * 6, 1, 0, 0 , 0);


	/*--------------------------------------
		使用していないリソースメモリに記録する
	--------------------------------------*/

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = indexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = vertexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = materialResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = transformationMatrixResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = directionalLightResource;
			break;
		}
	}
}

// モデルを描画する
void Engine::DrawModel(uint32_t modelHandle ,Transform3D& transform, const Matrix4x4& viewProjectionMatrix, const DirectionalLight& light)
{
	// ビューポートの設定
	commands_->GetCommandList()->RSSetViewports(1, &viewport_);

	// シザーの設定
	commands_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);

	// rootSignature
	commands_->GetCommandList()->SetGraphicsRootSignature(rootSignature_);

	// PSOの設定
	commands_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());


	// 頂点リソースを作成する
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = 
		CreateBufferResource(device_,UINT(sizeof(VertexData) * modelManager_->GetModelData(modelHandle).vertices.size()));
	
	// VBVを設定する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelManager_->GetModelData(modelHandle).vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelManager_->GetModelData(modelHandle).vertices.data(),
		sizeof(VertexData) * modelManager_->GetModelData(modelHandle).vertices.size());
	


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device_, sizeof(Material));

	// マテリアルに書き込むデータ
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	Transform3D uvTransform = { {1.0f , 1.0f , 1.0f} , {0.0f , 0.0f , 0.0f} , {0.0f , 0.0f , 0.0f} };

	materialData->color = { 1.0f , 1.0f , 1.0f , 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = Multiply(Multiply(Make4x4ScaleMatrix(uvTransform.scale),
		Make4x4RotateZMatrix(uvTransform.rotate.z)), Make4x4TranslateMatrix(uvTransform.translate));


	// 座標変換用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = CreateBufferResource(device_, sizeof(TransformationMatrix));

	// 行列に書き込む
	TransformationMatrix* transformationMatrixData = nullptr;
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->world = Make4x4AffineMatrix(transform.scale, transform.rotate, transform.translate);
	transformationMatrixData->worldViewProjection = Multiply(transformationMatrixData->world, viewProjectionMatrix);


	// 平行光源用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device_, sizeof(DirectionalLight));

	// データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = light.color;
	directionalLightData->direction = light.direction;
	directionalLightData->intensity = light.intensity;



	// VBVを設定する
	commands_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 形状を設定
	commands_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標変換用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// 平行光源用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	// テクスチャのCBVを設定する
	textureManager_->SelectTexture(modelManager_->GetTextureNumber(modelHandle), commands_->GetCommandList());

	// 描画する
	commands_->GetCommandList()->DrawInstanced(UINT(modelManager_->GetModelData(modelHandle).vertices.size()), 1, 0, 0);


	/*--------------------------------------
		使用していないリソースメモリに記録する
	--------------------------------------*/

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = vertexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = materialResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = transformationMatrixResource;
			break;
		}
	}

	for (uint32_t i = 0; i < kNumResourceMemories; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = directionalLightResource;
			break;
		}
	}
}