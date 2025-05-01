#include "DrnPCH.h"
#include "MeshTypes.h"

namespace Drn
{
	void MaterialData::Serialize( Archive& Ar )
	{
	
		if ( Ar.IsLoading() )
		{
			Ar >> m_Name;
	
			std::string MaterialPath;
			Ar >> MaterialPath;
			m_Material = AssetHandle<Material>( MaterialPath );
			m_Material.Load();
		}
		else
		{
			Ar << m_Name;
			Ar << m_Material.GetPath();
		}
	}
}