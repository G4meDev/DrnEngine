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

	Archive& Archive::operator>>( BoxSphereBounds& Value )
	{
		*this >> Value.Origin >> Value.BoxExtent >> Value.SphereRadius;
		return *this;
	}

        }  // namespace Drn