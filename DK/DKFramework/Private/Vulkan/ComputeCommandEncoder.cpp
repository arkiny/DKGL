//
//  File: ComputeCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "ComputeCommandEncoder.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

ComputeCommandEncoder::ComputeCommandEncoder(VkCommandBuffer vcb, CommandBuffer* cb)
	: encodingBuffer(vcb)
	, commandBuffer(cb)
{
}

ComputeCommandEncoder::~ComputeCommandEncoder(void)
{
}

void ComputeCommandEncoder::EndEncoding(void)
{
}

DKCommandBuffer* ComputeCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_USE_VULKAN
