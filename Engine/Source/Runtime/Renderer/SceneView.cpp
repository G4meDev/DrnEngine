#include "DrnPCH.h"
#include "SceneView.h"

namespace Drn
{
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