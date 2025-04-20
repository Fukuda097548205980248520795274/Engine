#include "Get.h"

/// <summary>
/// DXGIfactoryを取得する
/// </summary>
/// <returns></returns>
Microsoft::WRL::ComPtr<IDXGIFactory7> GetDXGIFactory()
{
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	return dxgiFactory;
}

/// <summary>
/// 使用するアダプタ（GPU）を取得する
/// </summary>
/// <param name="dxgiFactory"></param>
/// <returns></returns>
Microsoft::WRL::ComPtr<IDXGIAdapter4> GetUseAdapter(std::ostream& os, Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory)
{
	// 使用するアダプタ（GPU）
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

	// 良い順にアダプタを頼む
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		// アダプター情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			// 採用したアダプタの情報をログに出力
			Log(os ,ConvertString(std::format(L"Use Adapter : {} \n", adapterDesc.Description)));
			break;
		}

		// ソフトウェアアダプタの場合は、取り消す
		useAdapter = nullptr;
	}

	assert(useAdapter != nullptr);

	return useAdapter;
}

/// <summary>
/// Deviceを取得する
/// </summary>
/// <param name="useAdapter">使用するアダプタ（GPU）</param>
/// <returns></returns>
Microsoft::WRL::ComPtr<ID3D12Device> GetDevice(std::ostream& os, Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter)
{
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

	// 機能レベル
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	// ログ出力用
	const char* featureLevelStrings[] = { "12.2" , "12.1" , "12.0" };

	// 高い順に生成できるか確かめる
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		// 採用したアダプタででデバイスを生成する
		HRESULT hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		
		// 指定した機能レベルでDeviceを生成できたかを確認する
		if (SUCCEEDED(hr))
		{
			// ログを出力する
			Log(os,std::format("FeatureLevel : {} \n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device != nullptr);

	return device;
}

/// <summary>
/// 
/// </summary>
/// <param name="descriptorHeap"></param>
/// <param name="descriptorSize"></param>
/// <param name="index"></param>
/// <returns></returns>
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

/// <summary>
/// 
/// </summary>
/// <param name="descriptorHeap"></param>
/// <param name="descriptorSize"></param>
/// <param name="index"></param>
/// <returns></returns>
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}