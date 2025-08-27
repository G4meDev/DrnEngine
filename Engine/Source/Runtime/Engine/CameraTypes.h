#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	enum class ECameraProjectionMode : uint8_t
	{
		Perspective,
		Orthographic
	};

	struct ViewInfo
	{
		ViewInfo();

		Vector Location;
		Quat Rotation;

		float FOV;
		float OrthoWidth;

		float NearClipPlane;
		float FarClipPlane;

		float AspectRatio;
		ECameraProjectionMode ProjectionMode;

		Matrix CalculateViewMatrix() const;
		Matrix CalculateProjectionMatrix() const;
	};
}