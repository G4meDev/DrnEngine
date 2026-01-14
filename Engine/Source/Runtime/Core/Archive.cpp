#include "DrnPCH.h"
#include "Archive.h"

LOG_DEFINE_CATEGORY( LogArchive, "Archive" );

namespace Drn
{
	Archive::Archive( bool InIsLoading)
		: m_IsLoading(InIsLoading)
		, m_ValidArchive(false)
	{}

	Archive::~Archive()
	{}


	Archive& Archive::operator<<( const BoxSphereBounds& Value )
	{
		*this << Value.Origin << Value.BoxExtent << Value.SphereRadius;
		return *this;
	}

	Archive& Archive::operator<<( const Matrix& Value )
	{
		*this << Value.m_Matrix._11 << Value.m_Matrix._12 << Value.m_Matrix._13 << Value.m_Matrix._14;
		*this << Value.m_Matrix._21 << Value.m_Matrix._22 << Value.m_Matrix._23 << Value.m_Matrix._24;
		*this << Value.m_Matrix._31 << Value.m_Matrix._32 << Value.m_Matrix._33 << Value.m_Matrix._34;
		*this << Value.m_Matrix._41 << Value.m_Matrix._42 << Value.m_Matrix._43 << Value.m_Matrix._44;

		return *this;
	}

	Archive& Archive::operator>>( Matrix& Value )
	{
		*this >> Value.m_Matrix._11 >> Value.m_Matrix._12 >> Value.m_Matrix._13 >> Value.m_Matrix._14;
		*this >> Value.m_Matrix._21 >> Value.m_Matrix._22 >> Value.m_Matrix._23 >> Value.m_Matrix._24;
		*this >> Value.m_Matrix._31 >> Value.m_Matrix._32 >> Value.m_Matrix._33 >> Value.m_Matrix._34;
		*this >> Value.m_Matrix._41 >> Value.m_Matrix._42 >> Value.m_Matrix._43 >> Value.m_Matrix._44;

		return *this;
	}

	Archive& Archive::operator>>( BoxSphereBounds& Value )
	{
		*this >> Value.Origin >> Value.BoxExtent >> Value.SphereRadius;
		return *this;
	}


        }  // namespace Drn