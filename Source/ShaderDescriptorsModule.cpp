#include "Globals.h"
#include "ShaderDescriptorsModule.h"
#include "Application.h"
#include "D3D12Module.h"


bool ShaderDescriptorsModule::init()
{
	device = app->getD3D12Module()->GetDevice();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1000;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap));
	return true;
}

bool ShaderDescriptorsModule::postInit()
{


	return true;
}

UINT ShaderDescriptorsModule::CreateSRV(ID3D12Resource* resource)
{
	device->CreateShaderResourceView(
		resource,
		nullptr,
		GetCPUHandle(nextFreeDescriptorIndex)
	);
	nextFreeDescriptorIndex++;
	return nextFreeDescriptorIndex - 1;
}

void ShaderDescriptorsModule::Allocate()
{

}

void ShaderDescriptorsModule::Reset()
{
	nextFreeDescriptorIndex = 0;
}


