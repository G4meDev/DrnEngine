#include "DrnPCH.h"
#include "RenderTexture.h"

namespace Drn
{
	const ClearValueBinding ClearValueBinding::None(EClearBinding::ENoneBound);
	const ClearValueBinding ClearValueBinding::Black(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::BlackMaxAlpha(Vector4(0.0f, 0.0f, 0.0f, FLT_MAX));
	const ClearValueBinding ClearValueBinding::White(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::Transparent(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	const ClearValueBinding ClearValueBinding::DepthOne(1.0f, 0);
	const ClearValueBinding ClearValueBinding::DepthZero(0.0f, 0);
	const ClearValueBinding ClearValueBinding::DepthNear(1, 0);
	const ClearValueBinding ClearValueBinding::DepthFar(0, 0);
	const ClearValueBinding ClearValueBinding::Green(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::DefaultNormal8Bit(Vector4(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f));

}