//
//  File: CommandBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "CommandBuffer.h"
#include "RenderCommandEncoder.h"
#include "ComputeCommandEncoder.h"
#include "BlitCommandEncoder.h"
#include "GraphicsDevice.h"
#include "CommandQueue.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

CommandBuffer::CommandBuffer(VkCommandPool p, DKCommandQueue* q)
	: commandPool(p)
	, queue(q)
{
	DKASSERT_DEBUG(commandPool);
}

CommandBuffer::~CommandBuffer(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

	vkDestroyCommandPool(device, commandPool, nullptr);
}

DKObject<DKRenderCommandEncoder> CommandBuffer::CreateRenderCommandEncoder(const DKRenderPassDescriptor& rp)
{
	VkCommandBuffer buffer = GetEncodingBuffer();
	if (buffer)
	{
		DKObject<RenderCommandEncoder> encoder = DKOBJECT_NEW RenderCommandEncoder(buffer, this, rp);
		return encoder.SafeCast<DKRenderCommandEncoder>();
	}
	return nullptr;
}

DKObject<DKComputeCommandEncoder> CommandBuffer::CreateComputeCommandEncoder(void)
{
	VkCommandBuffer buffer = GetEncodingBuffer();
	if (buffer)
	{
		DKObject<ComputeCommandEncoder> encoder = DKOBJECT_NEW ComputeCommandEncoder(buffer, this);
		return encoder.SafeCast<DKComputeCommandEncoder>();
	}
	return nullptr;
}

DKObject<DKBlitCommandEncoder> CommandBuffer::CreateBlitCommandEncoder(void)
{
	VkCommandBuffer buffer = GetEncodingBuffer();
	if (buffer)
	{
		DKObject<BlitCommandEncoder> encoder = DKOBJECT_NEW BlitCommandEncoder(buffer, this);
		return encoder.SafeCast<DKBlitCommandEncoder>();
	}
	return nullptr;
}

bool CommandBuffer::Commit(void)
{
	if (finishedBuffers.Count() > 0)
	{
		CommandQueue* commandQueue = this->queue.StaticCast<CommandQueue>();

		VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submitInfo.commandBufferCount = static_cast<uint32_t>(finishedBuffers.Count());
		submitInfo.pCommandBuffers = finishedBuffers;
		VkResult err = vkQueueSubmit(commandQueue->queue, 1, &submitInfo, nullptr);

		if (err != VK_SUCCESS)
		{
			DKLogE("ERROR: vkEndCommandBuffer failed: %s", VkResultCStr(err));
			DKASSERT(err == VK_SUCCESS);
		}

		finishedBuffers.Clear();
	}
	return false;
}

bool CommandBuffer::WaitUntilCompleted(void)
{
	return false;
}

VkCommandBuffer CommandBuffer::GetEncodingBuffer(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

	VkCommandBufferAllocateInfo  bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	bufferInfo.commandPool = commandPool;
	bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferInfo.commandBufferCount = 1;

	VkCommandBuffer buffer = nullptr;
	VkResult err = vkAllocateCommandBuffers(device, &bufferInfo, &buffer);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkAllocateCommandBuffers failed: %s", VkResultCStr(err));
		return NULL;
	}
	return buffer;
}

void CommandBuffer::FinishCommandBuffer(VkCommandBuffer cb)
{
	finishedBuffers.Add(cb);
}

#endif //#if DKGL_USE_VULKAN
