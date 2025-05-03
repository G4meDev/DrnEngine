#include "DrnPCH.h"
#include "Texture2D.h"

namespace Drn
{
	Texture2D::Texture2D( const std::string& InPath )
		: Texture(InPath)
	{
		
	}

#if WITH_EDITOR
	Texture2D::Texture2D( const std::string& InPath, const std::string& InSourcePath )
		: Texture(InPath)
	{
		
	}
#endif
 
	Texture2D::~Texture2D()
	{
		
	}

#if WITH_EDITOR
 void Texture2D::OpenAssetPreview()
	{
		
	}

	void Texture2D::CloseAssetPreview()
	{
		
	}
#endif
}