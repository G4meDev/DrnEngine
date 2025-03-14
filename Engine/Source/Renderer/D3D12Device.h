#pragma once

class D3D12Adapter;

class D3D12Device
{
public:
	D3D12Device(D3D12Adapter* InAdapter);

	inline D3D12Adapter* GetAdapter(){ return Adapter; }
	inline IDXGIDevice* GetDevice(){ return Device.Get(); }

	void InitializeDevice();

protected:

	D3D12Adapter* Adapter;
	Microsoft::WRL::ComPtr<IDXGIDevice> Device;

private:
};