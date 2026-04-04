#include "DrnPCH.h"
#include "SceneView.h"

namespace Drn
{
	float SceneRendererView::ConvertFromDeviceZ( float DeviceZ ) const
	{
		return DeviceZ * InvDeviceZToWorldZTransform.GetX() + InvDeviceZToWorldZTransform.GetY() + 1.0f / (DeviceZ * InvDeviceZToWorldZTransform.GetZ() - InvDeviceZToWorldZTransform.GetW());

		//Vector4 P = ProjectionToView.TransformVector4(Vector4(0, 0, DeviceZ, 1));
		//return P.GetZ() / P.GetW();

	}

	float SceneRendererView::ConvertToDeviceZ( float SceneDepth ) const
	{
		//[branch]
		//if (View.ViewToClip[3][3] < 1.0f)
		//{
			// Perspective
			//return 1.0f / ((SceneDepth + InvDeviceZToWorldZTransform.GetW()) * InvDeviceZToWorldZTransform.GetZ());

			//Vector4 P = ViewToProjection.TransformVector4(Vector4(0, 0, SceneDepth, 1));
			//return P.GetZ() / P.GetW();
		
			return ((1 / SceneDepth) + InvDeviceZToWorldZTransform.GetW()) / InvDeviceZToWorldZTransform.GetZ();
		
		//}
		//else
		//{
		//	// Ortho
		//    return SceneDepth * View.ViewToClip[2][2] + View.ViewToClip[3][2];
		//}
	}

	Vector4 SceneRendererView::PixelToScreen( float InX, float InY, float InZ ) const
	{
		return Vector4((InX / Size.X) * 2.0f - 1.0f, (InY / Size.Y) * -2.0f + 1.0f, InZ, 1);
	}
	
	Vector SceneRendererView::PixelToWorld( float InX, float InY, float InZ ) const
	{
		return ScreenToWorld(PixelToScreen(InX, InY, InZ));
	}
	
	Vector SceneRendererView::ScreenToWorld( const Vector4& InScreenPosition ) const
	{
		Vector4 Result = ProjectionToWorld.TransformVector4(InScreenPosition);
		return Vector(Result.GetX(), Result.GetY(), Result.GetZ()) / Result.GetW();
		//return Vector(Result.GetX(), Result.GetY(), Result.GetY());
	}
}