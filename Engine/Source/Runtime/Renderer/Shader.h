#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/VertexDeclaration.h"

namespace Drn
{
	class Shader : public SimpleRenderResource
	{
	public:

		Shader(EShaderType InShaderType)
			: SimpleRenderResource()
			, ShaderType(InShaderType)
		{}

		EShaderType ShaderType;
		D3D12_SHADER_BYTECODE ByteCode;
	};

	class VertexShader : public Shader
	{
	public:
		VertexShader() : Shader(EShaderType::Vertex) {}
	};

	class HullShader : public Shader
	{
	public:
		HullShader() : Shader(EShaderType::Hull) {}
	};

	class DomainShader : public Shader
	{
	public:
		DomainShader() : Shader(EShaderType::Domain) {}
	};

	class PixelShader : public Shader
	{
	public:
		PixelShader() : Shader(EShaderType::Pixel) {}
	};

	class GeometryShader : public Shader
	{
	public:
		GeometryShader() : Shader(EShaderType::Geometry) {}
	};

	class ComputeShader : public Shader
	{
	public:
		ComputeShader() : Shader(EShaderType::Compute) {}
	};

	struct BoundShaderStateInput
	{
		inline BoundShaderStateInput() {}

		inline BoundShaderStateInput
		(
			VertexDeclaration* InVertexDeclaration
			, VertexShader* InVertexShader
			, HullShader* InHullShader
			, DomainShader* InDomainShader
			, PixelShader* InPixelShader
			, GeometryShader* InGeometryShader
		)
			: m_VertexDeclaration(InVertexDeclaration)
			, m_VertexShader(InVertexShader)
			, m_HullShader(InHullShader)
			, m_DomainShader(InDomainShader)
			, m_PixelShader(InPixelShader)
			, m_GeometryShader(InGeometryShader)
		{}

		//void AddRefResources()
		//{
		//	drn_check(m_VertexDeclaration);
		//	m_VertexDeclaration->AddRef();
		//
		//	drn_check(m_VertexShader);
		//	m_VertexShader->AddRef();
		//
		//	if (m_HullShader)
		//	{
		//		m_HullShader->AddRef();
		//	}
		//
		//	if (m_DomainShader)
		//	{
		//		m_DomainShader->AddRef();
		//	}
		//
		//	if (m_PixelShader)
		//	{
		//		m_PixelShader->AddRef();
		//	}
		//
		//	if (m_GeometryShader)
		//	{
		//		m_GeometryShader->AddRef();
		//	}
		//}

		//void ReleaseResources()
		//{
		//	drn_check(m_VertexDeclaration);
		//	m_VertexDeclaration->Release();
		//
		//	drn_check(m_VertexShader);
		//	m_VertexShader->Release();
		//
		//	if (m_HullShader)
		//	{
		//		m_HullShader->Release();
		//	}
		//
		//	if (m_DomainShader)
		//	{
		//		m_DomainShader->Release();
		//	}
		//
		//	if (m_PixelShader)
		//	{
		//		m_PixelShader->Release();
		//	}
		//
		//	if (m_GeometryShader)
		//	{
		//		m_GeometryShader->Release();
		//	}
		//}

		TRefCountPtr<VertexDeclaration> m_VertexDeclaration;
		TRefCountPtr<VertexShader> m_VertexShader;
		TRefCountPtr<HullShader> m_HullShader;
		TRefCountPtr<DomainShader> m_DomainShader;
		TRefCountPtr<PixelShader> m_PixelShader;
		TRefCountPtr<GeometryShader> m_GeometryShader;
	};



	struct ShaderPermutationBool
	{
		
	};

	struct ShaderPermutationInt
	{
		
	};



	class GlobalShader : public Shader
	{

	};
}