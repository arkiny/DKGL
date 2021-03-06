//
//  File: DKGraphicsDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include "DKGraphicsDevice.h"
#include "Interface/DKGraphicsDeviceInterface.h"

using namespace DKFramework;

DKGraphicsDevice::DKGraphicsDevice()
{
	impl = DKGraphicsDeviceInterface::CreateInterface();
	if (impl == NULL)
		DKError::RaiseException("FATAL ERROR: Cannot create graphics device!");
}

DKGraphicsDevice::~DKGraphicsDevice()
{
	delete impl;
}

DKObject<DKCommandQueue> DKGraphicsDevice::CreateCommandQueue(uint32_t queueTypeFlags)
{
	return impl->CreateCommandQueue(this, queueTypeFlags);
}

DKObject<DKShaderModule> DKGraphicsDevice::CreateShaderModule(DKShader* shader)
{
	if (shader)
	{
		return impl->CreateShaderModule(this, shader);
	}
	return NULL;
}

DKObject<DKShaderBindingSet> DKGraphicsDevice::CreateShaderBindingSet(const DKShaderBindingSetLayout& layout)
{
    return impl->CreateShaderBindingSet(this, layout);
}

DKObject<DKRenderPipelineState> DKGraphicsDevice::CreateRenderPipeline(const DKRenderPipelineDescriptor& desc, DKPipelineReflection* reflection)
{
	return impl->CreateRenderPipeline(this, desc, reflection);
}

DKObject<DKComputePipelineState> DKGraphicsDevice::CreateComputePipeline(const DKComputePipelineDescriptor& desc, DKPipelineReflection* reflection)
{
	return impl->CreateComputePipeline(this, desc, reflection);
}

DKObject<DKGpuBuffer> DKGraphicsDevice::CreateBuffer(size_t size, DKGpuBuffer::StorageMode storage, DKCpuCacheMode cache)
{
	return impl->CreateBuffer(this, size, storage, cache);
}

DKObject<DKTexture> DKGraphicsDevice::CreateTexture(const DKTextureDescriptor& d)
{
	return impl->CreateTexture(this, d);
}

DKObject<DKSamplerState> DKGraphicsDevice::CreateSamplerState(const DKSamplerDescriptor& d)
{
    return impl->CreateSamplerState(this, d);
}

DKObject<DKGpuEvent> DKGraphicsDevice::CreateEvent()
{
    return impl->CreateEvent(this);
}

DKObject<DKGpuSemaphore> DKGraphicsDevice::CreateSemaphore()
{
    return impl->CreateSemaphore(this);
}
