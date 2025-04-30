#include "DrnPCH.h"
#include "VertexLayout.h"

namespace Drn
{
	D3D12_INPUT_ELEMENT_DESC VertexLayout::Color[2] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_ELEMENT_DESC VertexLayout::LineColorThickness[2] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//additional alpha channel for thickness
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexLayout::GetLayoutDescriptionForType( EInputLayoutType Type )
	{
		switch ( Type )
		{
		case EInputLayoutType::Color:				return { VertexLayout::Color, _countof( VertexLayout::Color) };
		case EInputLayoutType::LineColorThickness:	return { VertexLayout::LineColorThickness, _countof( VertexLayout::LineColorThickness) };
		default:									return { VertexLayout::Color, _countof( VertexLayout::Color) };
		}
	}

	std::string VertexLayout::GetNameForType( EInputLayoutType Type )
	{
		switch ( Type )
		{
		case EInputLayoutType::Color:				return "Color";
		case EInputLayoutType::LineColorThickness:	return "LineColorThickness";
		default:									return "Color";
		}
	}

}