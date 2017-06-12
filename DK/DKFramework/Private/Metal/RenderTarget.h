//
//  File: RenderTarget.h
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#import <Metal/Metal.h>

#include "../../DKRenderTarget.h"
#include "../../DKGraphicsDevice.h"
#include "TextureBaseT.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class RenderTarget : public TextureBaseT<DKRenderTarget>
			{
			public:
				RenderTarget(id<MTLTexture>, DKGraphicsDevice*);
				~RenderTarget(void);

			private:
				DKObject<DKGraphicsDevice> device;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_METAL
