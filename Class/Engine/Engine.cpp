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

	// フェンス
	delete fence_;

	// スワップチェーン
	delete swapChain_;
	
	// コマンドリスト
	delete commands_;

	// エラー検知
	delete errorDetection_;

	// ウィンドウ
	delete window_;

	// COMの終了
	CoUninitialize();
}

// 初期化
void Engine::Initialize(const int32_t kClientWidth , const int32_t kClientHeight)
{

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

	SetUnhandledExceptionFilter(ExportDump);

	// ウィンドウの生成と初期化
	window_ = new Window();
	window_->Initialize(kClientWidth,kClientHeight);

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

	// 出力結果をSRGBに変換して書き込む
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 2Dテクスチャ
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


	// ディスクリプタの先頭の取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	// 1つめ　先頭
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResource_[0].Get(), &rtvDesc_, rtvHandles_[0]);

	// 2つめ
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResource_[1].Get(), &rtvDesc_, rtvHandles_[1]);


	// Fenceの生成と初期化
	fence_ = new Fence();
	fence_->Initialize(device_);


	/*---------------
	    テクスチャ
	---------------*/

	// テクスチャ用のリソース
	DirectX::ScratchImage mipImages = LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource_ = CreateTextureResource(device_, metadata);
	UploadTextureData(textureResource_, mipImages);

	// metaDataを基にSRVを作成する
	srvDesc_.Format = metadata.format;
	srvDesc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc_.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc_.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVを作成するディスクリプタヒープの場所を決める
	textureSrvHandleCPU_ = srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	textureSrvHandleGPU_ = srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();

	// 先頭はImGuiが使っているので、その次を使う
	textureSrvHandleCPU_.ptr += device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU_.ptr += device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// SRVを生成する
	device_->CreateShaderResourceView(textureResource_.Get(), &srvDesc_, textureSrvHandleCPU_);




	/*-------------
	    描画設定
	-------------*/

	// シェーダーの初期化と生成
	shader_ = new Shader();
	shader_->Initialize();


	/*   RootSignature   */

	// PixelShader CBV 0
	rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters_[0].Descriptor.ShaderRegister = 0;

	// VertexShader CBV 0
	rootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters_[1].Descriptor.ShaderRegister = 0;

	// ルートシグネチャを設定する
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature_.pParameters = rootParameters_;
	descriptionRootSignature_.NumParameters = _countof(rootParameters_);

	// シリアライズにしてバイナリにする
	hr = D3D12SerializeRootSignature(&descriptionRootSignature_, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
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

	// float4 position : POSITION0;
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDescs_.pInputElementDescs = inputElementDescs_;
	inputLayoutDescs_.NumElements = _countof(inputElementDescs_);



	/*   BlendState   */

	// 全ての色要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;


	/*   RasterizeState   */

	// 裏面（時計回り）を表示しない
	rasterizeDesc_.CullMode = D3D12_CULL_MODE_BACK;

	// 三角形を塗りつぶす
	rasterizeDesc_.FillMode = D3D12_FILL_MODE_SOLID;


	/*   シェーダーをコンパイルする   */

	vertexShaderBlob_ = shader_->CompilerShader(logStream,L"./Class/Engine/Shader/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = shader_->CompilerShader(logStream,L"./Class/Engine/Shader/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob_ != nullptr);


	/*   全ての描画設定を詰め込む   */

	graphicsPipelineStateDesc_.pRootSignature = rootSignature_;
	graphicsPipelineStateDesc_.InputLayout = inputLayoutDescs_;
	graphicsPipelineStateDesc_.VS = { vertexShaderBlob_->GetBufferPointer() , vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc_.PS = { pixelShaderBlob_->GetBufferPointer() , pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc_.BlendState = blendDesc_;
	graphicsPipelineStateDesc_.RasterizerState = rasterizeDesc_;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc_.NumRenderTargets = 1;
	graphicsPipelineStateDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ（形状）のタイプ
	graphicsPipelineStateDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように色を打ち込むかの設定
	graphicsPipelineStateDesc_.SampleDesc.Count = 1;
	graphicsPipelineStateDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 設定を基に生成する
	hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_, IID_PPV_ARGS(&graphicsPipelineState_));
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
	ImGui_ImplDX12_Init(device_.Get(), swapChain_->GetSwapChainDesc().BufferCount, rtvDesc_.Format,
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

	// バックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// バックバッファの状態を Present -> RenderTarget に遷移させる
	TransitionBarrier(commands_->GetCommandList(), swapChainResource_[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 描画先のRTVを設定する
	commands_->GetCommandList()->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, nullptr);

	// 指定した色で画面をクリアする
	float clearColor[] = { 0.1f , 0.25f , 0.5f , 1.0f };
	commands_->GetCommandList()->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);

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
}

// 三角形を描画する
void Engine::DrawTriangle(struct Transform3D& transform, const Matrix4x4& viewProjectionMatrix, float red, float green, float blue, float alpha)
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
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device_, sizeof(VertexData) * 3);

	// VBVを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 3;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// データを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { 0.0f , 0.5f , 0.0f , 1.0f };
	vertexData[0].texcoord = { 0.5f , 0.0f };
	vertexData[1].position = { 0.5f , -0.5f , 0.0f , 1.0f };
	vertexData[0].texcoord = { 1.0f , 1.0f };
	vertexData[2].position= { -0.5f , -0.5f , 0.0f , 1.0f };
	vertexData[2].texcoord = { 0.0f , 1.0f };


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device_, sizeof(Vector4));

	// マテリアルに書き込むデータ
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = Vector4(red, green, blue, alpha);


	// 座標変換用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> worldViewProjectionResource = CreateBufferResource(device_, sizeof(Matrix4x4));

	// ワールド行列
	Matrix4x4 worldMatrix = Make4x4AffineMatrix(transform.scale, transform.rotate, transform.translate);

	// 行列に書き込む
	Matrix4x4* worldViewProjectionData = nullptr;
	worldViewProjectionResource->Map(0, nullptr, reinterpret_cast<void**>(&worldViewProjectionData));
	*worldViewProjectionData = Multiply(worldMatrix, viewProjectionMatrix);


	// VBVを設定する
	commands_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 形状を設定
	commands_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標変換用のCBVを設定する
	commands_->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldViewProjectionResource->GetGPUVirtualAddress());

	// 描画する
	commands_->GetCommandList()->DrawInstanced(3, 1, 0, 0);

	
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
			resourceMemories[i] = worldViewProjectionResource;
			break;
		}
	}
}