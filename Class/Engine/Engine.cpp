#include "Engine.h"

// デストラクタ
Engine::~Engine()
{
	for (uint32_t i = 0; i < 128; i++)
	{
		if (resourceMemories[i])
		{
			resourceMemories[i]->Release();
		}
	}

	graphicsPipelineState_->Release();
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

	swapChainResource_[0]->Release();
	swapChainResource_[1]->Release();

	// スワップチェーン
	delete swapChain_;

	rtvDescriptorHeap_->Release();
	
	// コマンドリスト
	delete commands_;

	device_->Release();
	useAdapter_->Release();
	dxgiFactory_->Release();

	// エラー検知
	delete errorDetection_;

	// ウィンドウ
	delete window_;


	// リソースリークチェッカー
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
	{
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
}

// 初期化
void Engine::Initialize(const int32_t kClientWidth , const int32_t kClientHeight)
{

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
	useAdapter_ = GetUseAdapter(dxgiFactory_);

	// Deviceを取得する
	device_ = GetDevice(useAdapter_);

	// 初期化完了!!!
	Log("Complate create ID3D12Device!! \n");


	// エラーを検知したら停止する
	errorDetection_->MakeItStop(device_);


	// コマンドの生成と初期化
	commands_ = new Commands();
	commands_->Initialize(device_);


	/*----------------------------
	    DescriptorHeapを生成する
	----------------------------*/

	// RTV用のディスクリプタ
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numRtvDescriptor_, false);


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
	device_->CreateRenderTargetView(swapChainResource_[0], &rtvDesc_ , rtvHandles_[0]);

	// 2つめ
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResource_[1], &rtvDesc_, rtvHandles_[1]);


	// Fenceの生成と初期化
	fence_ = new Fence();
	fence_->Initialize(device_);


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
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
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

	vertexShaderBlob_ = shader_->CompilerShader(L"./Class/Engine/Shader/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = shader_->CompilerShader(L"./Class/Engine/Shader/Object3D.PS.hlsl", L"ps_6_0");
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

// 描画前処理
void Engine::preDraw()
{
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

}

// 描画後処理
void Engine::postDraw()
{
	// バックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// バックバッファの状態を RenderTarget -> Present に遷移させる
	TransitionBarrier(commands_->GetCommandList(), swapChainResource_[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// コマンドリストの内容を確定させる
	HRESULT hr = commands_->GetCommandList()->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commands_->GetCommandList() };
	commands_->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// GPUとOSに画面の交換を行うように通知する
	swapChain_->GetSwapChain()->Present(1, 0);

	// GPUの完了を待つ
	fence_->WaitForGPU(commands_->GetCommandQueue());

	// 次のフレーム用のコマンドリストを準備
	hr = commands_->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = commands_->GetCommandList()->Reset(commands_->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));


	// 使用したリソースのアドレスを、リリース用アドレスで開放する
	for (uint32_t i = 0; i < 128; i++)
	{
		if (resourceMemories[i])
		{
			ID3D12Resource* releaseResource = resourceMemories[i];
			resourceMemories[i] = nullptr;
			releaseResource->Release();
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
	commands_->GetCommandList()->SetPipelineState(graphicsPipelineState_);


	// 頂点リソースを作る
	ID3D12Resource* vertexResource = CreateBufferResource(device_, sizeof(Vector4) * 3);

	// VBVを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// データを書き込む
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = { 0.0f , 0.5f , 0.0f , 1.0f };
	vertexData[1] = { 0.5f , -0.5f , 0.0f , 1.0f };
	vertexData[2] = { -0.5f , -0.5f , 0.0f , 1.0f };


	// マテリアル用のリソースを作る
	ID3D12Resource* materialResource = CreateBufferResource(device_, sizeof(Vector4));

	// マテリアルに書き込むデータ
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = Vector4(red, green, blue, alpha);


	// 座標変換用のリソース
	ID3D12Resource* worldViewProjectionResource = CreateBufferResource(device_, sizeof(Matrix4x4));

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

	for (uint32_t i = 0; i < 128; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = vertexResource;
			break;
		}
	}

	for (uint32_t i = 0; i < 128; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = materialResource;
			break;
		}
	}

	for (uint32_t i = 0; i < 128; i++)
	{
		if (resourceMemories[i] == nullptr)
		{
			resourceMemories[i] = worldViewProjectionResource;
			break;
		}
	}
}