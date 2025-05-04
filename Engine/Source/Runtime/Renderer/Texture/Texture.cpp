#include "DrnPCH.h"
#include "Texture.h"

namespace Drn
{
	void Texture::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_SourcePath;
			Ar >> m_sRGB;
			Ar >> m_SizeX;
			Ar >> m_SizeY;
			Ar >> m_MipLevels;
			uint8 Format;
			Ar >> Format;
			m_Format = static_cast<DXGI_FORMAT>(Format);
			Ar >> m_RowPitch;
			Ar >> m_SlicePitch;
			Ar >> m_ImageBlob;
		}

		else
		{
			Ar << m_SourcePath;
			Ar << m_sRGB;
			Ar << m_SizeX;
			Ar << m_SizeY;
			Ar << m_MipLevels;
			Ar << static_cast<uint8>(m_Format);
			Ar << m_RowPitch;
			Ar << m_SlicePitch;
			Ar << m_ImageBlob;
		}
	}

}