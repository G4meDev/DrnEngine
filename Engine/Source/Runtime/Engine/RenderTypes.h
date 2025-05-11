#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class NamedTexture2DSlot : public Serializable
	{
	public:
		NamedTexture2DSlot()
			: m_Texture2D( DEFAULT_TEXTURE_PATH ) {};

		NamedTexture2DSlot(const std::string& Name, const std::string& TexturePath)
			: m_Name(Name)
			, m_Texture2D(TexturePath)
		{}

		virtual void Serialize( Archive& Ar ) override;

		std::string m_Name;
		AssetHandle<Texture2D> m_Texture2D;

#if WITH_EDITOR
		AssetHandle<Texture2D> Draw();
#endif
	};


}