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



}