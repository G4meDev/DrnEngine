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
	};

// -----------------------------------------------------------------------------------------------

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

	class MaterialIndexedTexture2DParameter : public Texture2DProperty
	{
	public:
		MaterialIndexedTexture2DParameter()
			: Texture2DProperty("", DEFAULT_TEXTURE_PATH)
			, m_Index(0)
		{
		};

		MaterialIndexedTexture2DParameter(const std::string& Name, const std::string& TexturePath, uint16 Index)
			: Texture2DProperty(Name, TexturePath)
			, m_Index(Index)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		uint16 m_Index;
	};

// -----------------------------------------------------------------------------------------------

	class TextureCubeProperty : public NamedProperty
	{
	public:
		TextureCubeProperty()
			: NamedProperty("")
			, m_TextureCube( "" )
		{
		};

		TextureCubeProperty( const std::string& Name, const std::string& TexturePath )
			: NamedProperty(Name)
			, m_TextureCube(TexturePath)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		AssetHandle<TextureCube> m_TextureCube;

#if WITH_EDITOR
		AssetHandle<TextureCube> Draw();
#endif
	};

	class MaterialIndexedTextureCubeParameter : public TextureCubeProperty
	{
	public:
		MaterialIndexedTextureCubeParameter()
			: TextureCubeProperty("", "")
			, m_Index(0)
		{
		};

		MaterialIndexedTextureCubeParameter(const std::string& Name, const std::string& TexturePath, uint16 Index)
			: TextureCubeProperty(Name, TexturePath)
			, m_Index(Index)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		uint16 m_Index;
	};

// -----------------------------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------------------------

	class Vector4Property : public NamedProperty
	{
	public:
		Vector4Property()
			: NamedProperty("")
			, m_Value()
		{
		};

		Vector4Property(const std::string& Name, const Vector4& Value)
			: NamedProperty(Name)
			, m_Value(Value)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		Vector4 m_Value;

#if WITH_EDITOR
		bool Draw();
#endif
	};
	

	// TODO: maybe multiple inheritance
	class MaterialIndexedVector4Parameter : public Vector4Property
	{
	public:
		MaterialIndexedVector4Parameter()
			: Vector4Property()
			, m_Index(0)
		{
		};

		MaterialIndexedVector4Parameter(const std::string& Name, const Vector4& Value, uint16 Index)
			: Vector4Property(Name, Value)
			, m_Index(Index)
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		uint16 m_Index;
	};
}