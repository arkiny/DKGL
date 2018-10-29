//
//  File: GraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"

#include "../../Interface/DKGraphicsDeviceInterface.h"
#include "QueueFamily.h"

namespace DKFramework::Private::Vulkan
{
	class GraphicsDevice : public DKGraphicsDeviceInterface
	{
	public:
		GraphicsDevice();
		~GraphicsDevice();

		DKString DeviceName() const override;
		DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*, uint32_t) override;
		DKObject<DKShaderModule> CreateShaderModule(DKGraphicsDevice*, DKShader*) override;
		DKObject<DKRenderPipelineState> CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*) override;
		DKObject<DKComputePipelineState> CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*) override;
		DKObject<DKGpuBuffer> CreateBuffer(DKGraphicsDevice*, size_t, DKGpuBuffer::StorageMode, DKCpuCacheMode) override;
		DKObject<DKTexture> CreateTexture(DKGraphicsDevice*, const DKTextureDescriptor&) override;

		VkFence GetFence();
		void AddFenceCompletionHandler(VkFence, DKOperation*, bool useEventLoop = false);

		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;

		DKAtomicNumber64 numberOfFences;
		DKArray<QueueFamily*> queueFamilies;

		// device properties
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		DKArray<VkMemoryType> deviceMemoryTypes;
		DKArray<VkMemoryHeap> deviceMemoryHeaps;
		DKArray<VkQueueFamilyProperties> queueFamilyProperties;
		DKArray<VkExtensionProperties> extensionProperties;

		bool enableValidation;
		VkDebugReportCallbackEXT msgCallback;

		struct FenceCallback
		{
			VkFence fence;
			DKObject<DKOperation> operation;
			DKThread::ThreadId threadId;
		};
		DKArray<FenceCallback> pendingFenceCallbacks;
		DKArray<VkFence> reusableFences;
		DKCondition fenceCompletionCond;
		bool fenceCompletionThreadRunning;
		DKObject<DKThread> fenceCompletionThread;
		void FenceCompletionCallbackThreadProc();

		VkPipelineCache pipelineCache;
		void LoadPipelineCache();
		void SavePipelineCache();

		// extensions
		InstanceProc iproc; // instance procedure
		DeviceProc dproc; // device procedure

    private:
        VkPipelineLayout CreatePipelineLayout(std::initializer_list<const DKShaderFunction*>) const;
        VkPipelineLayout CreatePipelineLayout(std::initializer_list<const DKShaderFunction*>, DKArray<VkDescriptorSetLayout>&) const;
	};
}
#endif //#if DKGL_ENABLE_VULKAN
