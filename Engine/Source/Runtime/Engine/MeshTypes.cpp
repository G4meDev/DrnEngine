#include "DrnPCH.h"
#include "MeshTypes.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

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

	void MaterialOverrideData::Serialize( Archive& Ar )
	{
		MaterialData::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Overriden;
		}

		else
		{
			Ar << m_Overriden;
		}
	}

#if WITH_EDITOR
	void MaterialOverrideData::Draw( StaticMeshComponent* MC )
	{
		ImGui::Text("123123##123");
	}
#endif

}