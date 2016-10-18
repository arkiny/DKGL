//
//  File: DKGraphicsDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKGpuResource.h"
#include "DKGpuCommandBuffer.h"

////////////////////////////////////////////////////////////////////////////////
// DKGraphicsDevice
// Graphics Device
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGpuCommandQueue
	{
	public:
		virtual ~DKGpuCommandQueue(void) {}
		virtual DKObject<DKGpuCommandBuffer> CreateCommandBuffer(void) = 0;
	};

	class DKGraphicsDeviceInterface;
	class DKGL_API DKGraphicsDevice : public DKSharedInstance<DKGraphicsDevice>
	{
		DKGraphicsDevice(void);
		~DKGraphicsDevice(void);


		DKObject<DKGpuCommandQueue> CreateCommandQueue(void);

	private:
		DKGraphicsDeviceInterface* impl;
	};
}