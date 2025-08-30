#include "GamePCH.h"
#include "TestPlayerCharacter.h"

#include "Editor/Misc/EditorMisc.h"

#if WITH_EDITOR
#include <Imgui.h>
#endif

namespace Drn
{
	
	DECLARE_LEVEL_SPAWNABLE_CLASS( TestPlayerCharacter, Game );

	TestPlayerCharacter::TestPlayerCharacter()
		: Character()
	{
		
	}

	TestPlayerCharacter::~TestPlayerCharacter()
	{
		
	}

	void TestPlayerCharacter::Serialize( Archive& Ar )
	{
		Character::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Dummy;
		}

		else
		{
			Ar << m_Dummy;
		}
	}

#if WITH_EDITOR
	bool TestPlayerCharacter::DrawDetailPanel()
	{
		bool Dirty = Character::DrawDetailPanel();

		ImGui::InputFloat( "Dummy", &m_Dummy );

		return Dirty;
	}
#endif
}