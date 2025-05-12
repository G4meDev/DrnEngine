#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class NamedProperty : public Serializable
	{
	public:
		NamedProperty(const std::string& Name)
			: m_Name(Name)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		std::string m_Name;

//#if WITH_EDITOR
//
//#endif
	};

	class Texture2DProperty : public NamedProperty
	{
	public:
		Texture2DProperty()
			: NamedProperty("")
			, m_Texture2D( DEFAULT_TEXTURE_PATH ) 
		{
		};

		Texture2DProperty(const std::string& Name, const std::string& TexturePath)
			: NamedProperty(Name)
			, m_Texture2D(TexturePath)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		AssetHandle<Texture2D> m_Texture2D;

#if WITH_EDITOR
		AssetHandle<Texture2D> Draw();
#endif
	};

	class FloatProperty : public NamedProperty
	{
	public:
		FloatProperty()
			: NamedProperty("")
			, m_Value(0.0f)
		{
		};

		FloatProperty(const std::string& Name, float Value)
			: NamedProperty(Name)
			, m_Value(Value)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		float m_Value;

#if WITH_EDITOR
		bool Draw();
#endif
	};


	class MaterialIndexedFloatParameter : public FloatProperty
	{
	public:
		MaterialIndexedFloatParameter()
			: FloatProperty("", 0.0f)
			, m_Index(0)
		{
		};

		MaterialIndexedFloatParameter(const std::string& Name, float Value, uint16 Index)
			: FloatProperty(Name, Value)
			, m_Index(Index)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		uint16 m_Index;
	};

}