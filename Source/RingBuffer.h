#pragma once
#include <queue>
#include <d3d12.h>

struct AllocationInfo
{
    UINT frameIndex;
    size_t offset;
    size_t size;
};

class RingBuffer
{
public:
    RingBuffer(uint32_t sizeInMB);
    ~RingBuffer();

    D3D12_GPU_VIRTUAL_ADDRESS Allocate(const void* data, size_t size, UINT currentFrame);
    void Free(UINT lastCompletedFrame);
    void Reset();

    size_t GetTotalSize() const { return totalMemorySize; }

    ID3D12Resource* GetResource() const { return buffer.Get(); }

private:
    std::queue<AllocationInfo> allocationQueue;

    ComPtr<ID3D12Resource> buffer;
    uint8_t* mappedData = nullptr;
    size_t totalMemorySize = 0;
    size_t head = 0; //Oldest allocation
    size_t tail = 0; // Position of the next allocation
};