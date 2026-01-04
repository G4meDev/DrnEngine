#include "DrnPCH.h"
#include "MaterialShared.h"

namespace Drn
{
	void MaterialUniformParameters::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Clear();

			uint8 Texture2DCount;
			Ar >> Texture2DCount;
			m_Texture2DSlots.reserve(Texture2DCount);
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots.push_back({});
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount;
			Ar >> TextureCubeCount;
			m_TextureCubeSlots.reserve(TextureCubeCount);
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots.push_back({});
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount;
			Ar >> ScalarCount;
			m_FloatSlots.reserve(ScalarCount);
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots.push_back({});
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count;
			Ar >> Vector4Count;
			m_Vector4Slots.reserve(Vector4Count);
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots.push_back({});
				m_Vector4Slots[i].Serialize(Ar);
			}
		}

		else
		{
			uint8 Texture2DCount = m_Texture2DSlots.size();
			Ar << Texture2DCount;
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount = m_TextureCubeSlots.size();
			Ar << TextureCubeCount;
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount = m_FloatSlots.size();
			Ar << ScalarCount;
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count = m_Vector4Slots.size();
			Ar << Vector4Count;
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots[i].Serialize(Ar);
			}
		}
	}

	void MaterialUniformParameters::UploadResources( class D3D12CommandList* CommandList )
	{
		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			if ( m_Texture2DSlots[i].m_Texture2D.IsValid())
			{
				m_Texture2DSlots[i].m_Texture2D->UploadResources(CommandList);
			}
		}

		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			if (m_TextureCubeSlots[i].m_TextureCube.IsValid())
			{
				m_TextureCubeSlots[i].m_TextureCube->UploadResources(CommandList);
			}
		}

		const int32 Vector4SlotCount = m_Vector4Slots.size() * 4;
		const int32 ScalarSlotCount = m_FloatSlots.size();
		const int32 Texture2DSlotCount = m_Texture2DSlots.size() * 2;
		const int32 TextureCubeSlotCount = m_TextureCubeSlots.size() * 2;
		const int32 SlotCount = Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount + TextureCubeSlotCount;

		std::vector<uint32> Parameters;
		Parameters.resize(SlotCount);

		for (int i = 0; i < m_Vector4Slots.size(); i++)
		{
			*(Vector4*)(&Parameters[i * 4]) = m_Vector4Slots[i].m_Value;
		}

		for (int i = 0; i < m_FloatSlots.size(); i++)
		{
			*(float*)(&Parameters[i + Vector4SlotCount]) = m_FloatSlots[i].m_Value;
		}

		for (int i = 0; i < m_Texture2DSlots.size(); i++)
		{
			Parameters[i * 2 + Vector4SlotCount + ScalarSlotCount] = m_Texture2DSlots[i].m_Texture2D.IsValid() ? m_Texture2DSlots[i].m_Texture2D->GetTextureIndex() : 0;
			Parameters[i * 2 + 1 + Vector4SlotCount + ScalarSlotCount] = m_Texture2DSlots[i].m_Texture2D.IsValid() ? m_Texture2DSlots[i].m_Texture2D->GetSamplerIndex() : 0;
		}

		for (int i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			Parameters[i * 2 + Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount] = m_TextureCubeSlots[i].m_TextureCube.IsValid() ? m_TextureCubeSlots[i].m_TextureCube->GetTextureIndex() : 0;
			Parameters[i * 2 + 1 + Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount] = m_TextureCubeSlots[i].m_TextureCube.IsValid() ? m_TextureCubeSlots[i].m_TextureCube->GetSamplerIndex() : 0;
		}

		if ( Parameters.size() > 0 )
		{
			//uint32 Size = Align(TextureIndices.size() * sizeof(uint32), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			uint32 Size = Parameters.size() * sizeof(uint32);
			ParametersBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), Size, EUniformBufferUsage::SingleFrame, Parameters.data());
		}
	}

	void MaterialUniformParameters::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			const Texture2DProperty& TextureSlot = m_Texture2DSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTexture2D(i, TextureAsset);
				return;
			}
		}
	}

	void MaterialUniformParameters::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_Texture2DSlots.size())
		{
			m_Texture2DSlots[Index].m_Texture2D = TextureAsset;
		}
	}

	void MaterialUniformParameters::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			const TextureCubeProperty& TextureSlot = m_TextureCubeSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTextureCube(i, TextureAsset);
				return;
			}
		}
	}

	void MaterialUniformParameters::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_TextureCubeSlots.size())
		{
			m_TextureCubeSlots[Index].m_TextureCube = TextureAsset;
		}
	}

	void MaterialUniformParameters::SetIndexedScalar( uint32 Index, float Value )
	{
		if (Index >= 0 && Index < m_FloatSlots.size())
		{
			m_FloatSlots[Index].m_Value = Value;
		}
	}

	void MaterialUniformParameters::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		if (Index >= 0 && Index < m_Vector4Slots.size())
		{
			m_Vector4Slots[Index].m_Value = Value;
		}
	}

	void MaterialUniformParameters::SetNamedScalar( const std::string& Name, float Value )
	{
		for ( int32 i = 0; i < m_FloatSlots.size(); i++)
		{
			if (m_FloatSlots[i].m_Name == Name)
			{
				SetIndexedScalar(i, Value);
			}
		}
	}

	void MaterialUniformParameters::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		for ( int32 i = 0; i < m_Vector4Slots.size(); i++)
		{
			if (m_Vector4Slots[i].m_Name == Name)
			{
				SetIndexedVector(i, Value);
			}
		}
	}
}