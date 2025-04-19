#include "ErrorDetection.h"

// デストラクタ
ErrorDetection::~ErrorDetection()
{
	
}

// 初期化
void ErrorDetection::Initialize()
{
	/*------------------------------
	    デバッグレイヤーを有効化する
	------------------------------*/

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_))))
	{
		// デバッグレイヤーを有効化する
		debugController_->EnableDebugLayer();

		// GPU側も有効化する
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}
}

// エラーで停止させる
void ErrorDetection::MakeItStop(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		// やばいエラー時
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

		// エラー時
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		
		// 警告時
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		
		/*---------------------------
		    抑制するエラーを指定する
		---------------------------*/

		// 抑制するメッセージID
		D3D12_MESSAGE_ID denyIds[] =
		{
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージを抑制する
		infoQueue->PushStorageFilter(&filter);
	}
}