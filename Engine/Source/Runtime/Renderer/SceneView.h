#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct SceneRendererView
	{
		SceneRendererView()
		{
		}
		~SceneRendererView()
		{
		}

		Matrix WorldToView;
		Matrix ViewToProjection;
		Matrix WorldToProjection;
		Matrix ProjectionToView;
		Matrix ProjectionToWorld;
		Matrix LocalToCameraView;

		IntPoint Size;
		float InvSizeX;
		float InvSizeY;

		Vector CameraPos;
		float InvTanHalfFov;
		
		Vector CameraDir;
		float AspectRatio;

		Vector4 InvDeviceZToWorldZTransform;
		Matrix ViewToWorld;
		Matrix ScreenToTranslatedWorld;

		uint32 FrameIndex;
		uint32 FrameIndexMod8;
		float JitterOffset[2];

		float PrevJitterOffset[2];
		float Pad_1[2];

		Matrix ClipToPreviousClip;

		Vector4 PixelToScreen(float InX, float InY, float InZ) const;
		Vector PixelToWorld(float InX, float InY, float InZ) const;
		Vector ScreenToWorld(const Vector4& InScreenPosition) const;
	};
}