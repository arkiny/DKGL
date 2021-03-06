//
//  File: ComputeCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12

#include "ComputeCommandEncoder.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Direct3D;

ComputeCommandEncoder::ComputeCommandEncoder(ID3D12GraphicsCommandList* cm, class CommandBuffer* cb)
	: commandList(cm)
	, commandBuffer(cb)
{
	DKASSERT_DEBUG(commandList != nullptr);
}

ComputeCommandEncoder::~ComputeCommandEncoder()
{
}

void ComputeCommandEncoder::EndEncoding()
{
	if (commandList)
	{
		commandList->Close();

		commandBuffer->FinishCommandList(commandList.Get());
		commandList = nullptr;
	}
}

DKCommandBuffer* ComputeCommandEncoder::CommandBuffer()
{
	return commandBuffer;
}

bool ComputeCommandEncoder::IsCompleted() const
{
    return commandList == nullptr; 
}

void ComputeCommandEncoder::SetResources(uint32_t set, DKShaderBindingSet*)
{

}

void ComputeCommandEncoder::SetComputePipelineState(DKComputePipelineState*)
{

}

#endif //#if DKGL_ENABLE_DIRECT3D12
