#include "DrnPCH.h"
#include "ResourceView.h"

namespace Drn
{
	void BaseShaderResourceView::Remove()
	{
		if (DynamicResource)
		{
			DynamicResource->RemoveDynamicSRV(this);
		}
	}


	CSubresourceSubset::CSubresourceSubset( const CBufferView& )
		: m_BeginArray( 0 )
		, m_EndArray( 1 )
		, m_BeginMip( 0 )
		, m_EndMip( 1 )
		, m_BeginPlane( 0 )
		, m_EndPlane( 1 )
		{}

	CSubresourceSubset::CSubresourceSubset( const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat )
		: m_BeginArray( 0 )
		, m_EndArray( 1 )
		, m_BeginMip( 0 )
		, m_EndMip( 1 )
		, m_BeginPlane( 0 )
		, m_EndPlane( 1 )
		{
			switch ( Desc.ViewDimension )
			{
			default:
				break;

			case ( D3D12_SRV_DIMENSION_BUFFER ):
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE1D ):
				m_BeginMip   = uint8( Desc.Texture1D.MostDetailedMip );
				m_EndMip     = uint8( m_BeginMip + Desc.Texture1D.MipLevels );
				m_BeginPlane = GetPlaneSliceFromViewFormat( ResourceFormat, Desc.Format );
				m_EndPlane   = m_BeginPlane + 1;
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE1DARRAY ):
				m_BeginArray = uint16( Desc.Texture1DArray.FirstArraySlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture1DArray.ArraySize );
				m_BeginMip   = uint8( Desc.Texture1DArray.MostDetailedMip );
				m_EndMip     = uint8( m_BeginMip + Desc.Texture1DArray.MipLevels );
				m_BeginPlane = GetPlaneSliceFromViewFormat( ResourceFormat, Desc.Format );
				m_EndPlane   = m_BeginPlane + 1;
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE2D ):
				m_BeginMip   = uint8( Desc.Texture2D.MostDetailedMip );
				m_EndMip     = uint8( m_BeginMip + Desc.Texture2D.MipLevels );
				m_BeginPlane = uint8( Desc.Texture2D.PlaneSlice );
				m_EndPlane   = uint8( Desc.Texture2D.PlaneSlice + 1 );
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE2DARRAY ):
				m_BeginArray = uint16( Desc.Texture2DArray.FirstArraySlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture2DArray.ArraySize );
				m_BeginMip   = uint8( Desc.Texture2DArray.MostDetailedMip );
				m_EndMip     = uint8( m_BeginMip + Desc.Texture2DArray.MipLevels );
				m_BeginPlane = uint8( Desc.Texture2DArray.PlaneSlice );
				m_EndPlane   = uint8( Desc.Texture2DArray.PlaneSlice + 1 );
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE2DMS ):
				m_BeginPlane = GetPlaneSliceFromViewFormat( ResourceFormat, Desc.Format );
				m_EndPlane   = m_BeginPlane + 1;
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY ):
				m_BeginArray = uint16( Desc.Texture2DMSArray.FirstArraySlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture2DMSArray.ArraySize );
				m_BeginPlane = GetPlaneSliceFromViewFormat( ResourceFormat, Desc.Format );
				m_EndPlane   = m_BeginPlane + 1;
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURE3D ):
				m_EndArray = uint16( -1 );  // all slices
				m_BeginMip = uint8( Desc.Texture3D.MostDetailedMip );
				m_EndMip   = uint8( m_BeginMip + Desc.Texture3D.MipLevels );
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURECUBE ):
				m_BeginMip   = uint8( Desc.TextureCube.MostDetailedMip );
				m_EndMip     = uint8( m_BeginMip + Desc.TextureCube.MipLevels );
				m_BeginArray = 0;
				m_EndArray   = 6;
				m_BeginPlane = GetPlaneSliceFromViewFormat( ResourceFormat, Desc.Format );
				m_EndPlane   = m_BeginPlane + 1;
				break;

			case ( D3D12_SRV_DIMENSION_TEXTURECUBEARRAY ):
				m_BeginArray = uint16( Desc.TextureCubeArray.First2DArrayFace );
				m_EndArray   = uint16( m_BeginArray + Desc.TextureCubeArray.NumCubes * 6 );
				m_BeginMip   = uint8( Desc.TextureCubeArray.MostDetailedMip );
				m_EndMip     = uint8( m_BeginMip + Desc.TextureCubeArray.MipLevels );
				m_BeginPlane = GetPlaneSliceFromViewFormat( ResourceFormat, Desc.Format );
				m_EndPlane   = m_BeginPlane + 1;
				break;
			}
		}

	CSubresourceSubset::CSubresourceSubset( const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc )
		: m_BeginArray( 0 )
		, m_EndArray( 1 )
		, m_BeginMip( 0 )
		, m_BeginPlane( 0 )
		, m_EndPlane( 1 )
		{
			switch ( Desc.ViewDimension )
			{
			default:
					break;

			case ( D3D12_UAV_DIMENSION_BUFFER ):
					break;

			case ( D3D12_UAV_DIMENSION_TEXTURE1D ):
					m_BeginMip = uint8( Desc.Texture1D.MipSlice );
					break;

			case ( D3D12_UAV_DIMENSION_TEXTURE1DARRAY ):
					m_BeginArray = uint16( Desc.Texture1DArray.FirstArraySlice );
					m_EndArray   = uint16( m_BeginArray + Desc.Texture1DArray.ArraySize );
					m_BeginMip   = uint8( Desc.Texture1DArray.MipSlice );
					break;

			case ( D3D12_UAV_DIMENSION_TEXTURE2D ):
					m_BeginMip   = uint8( Desc.Texture2D.MipSlice );
					m_BeginPlane = uint8( Desc.Texture2D.PlaneSlice );
					m_EndPlane   = uint8( Desc.Texture2D.PlaneSlice + 1 );
					break;

			case ( D3D12_UAV_DIMENSION_TEXTURE2DARRAY ):
					m_BeginArray = uint16( Desc.Texture2DArray.FirstArraySlice );
					m_EndArray   = uint16( m_BeginArray + Desc.Texture2DArray.ArraySize );
					m_BeginMip   = uint8( Desc.Texture2DArray.MipSlice );
					m_BeginPlane = uint8( Desc.Texture2DArray.PlaneSlice );
					m_EndPlane   = uint8( Desc.Texture2DArray.PlaneSlice + 1 );
					break;

			case ( D3D12_UAV_DIMENSION_TEXTURE3D ):
					m_BeginArray = uint16( Desc.Texture3D.FirstWSlice );
					m_EndArray   = uint16( m_BeginArray + Desc.Texture3D.WSize );
					m_BeginMip   = uint8( Desc.Texture3D.MipSlice );
					break;
			}

			m_EndMip = m_BeginMip + 1;
		}

	CSubresourceSubset::CSubresourceSubset( const D3D12_RENDER_TARGET_VIEW_DESC& Desc )
		: m_BeginArray( 0 )
		, m_EndArray( 1 )
		, m_BeginMip( 0 )
		, m_BeginPlane( 0 )
		, m_EndPlane( 1 )
		{
			switch ( Desc.ViewDimension )
			{
			default:
				break;

			case ( D3D12_RTV_DIMENSION_BUFFER ):
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE1D ):
				m_BeginMip = uint8( Desc.Texture1D.MipSlice );
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE1DARRAY ):
				m_BeginArray = uint16( Desc.Texture1DArray.FirstArraySlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture1DArray.ArraySize );
				m_BeginMip   = uint8( Desc.Texture1DArray.MipSlice );
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE2D ):
				m_BeginMip   = uint8( Desc.Texture2D.MipSlice );
				m_BeginPlane = uint8( Desc.Texture2D.PlaneSlice );
				m_EndPlane   = uint8( Desc.Texture2D.PlaneSlice + 1 );
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE2DMS ):
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE2DARRAY ):
				m_BeginArray = uint16( Desc.Texture2DArray.FirstArraySlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture2DArray.ArraySize );
				m_BeginMip   = uint8( Desc.Texture2DArray.MipSlice );
				m_BeginPlane = uint8( Desc.Texture2DArray.PlaneSlice );
				m_EndPlane   = uint8( Desc.Texture2DArray.PlaneSlice + 1 );
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY ):
				m_BeginArray = uint16( Desc.Texture2DMSArray.FirstArraySlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture2DMSArray.ArraySize );
				break;

			case ( D3D12_RTV_DIMENSION_TEXTURE3D ):
				m_BeginArray = uint16( Desc.Texture3D.FirstWSlice );
				m_EndArray   = uint16( m_BeginArray + Desc.Texture3D.WSize );
				m_BeginMip   = uint8( Desc.Texture3D.MipSlice );
				break;
			}

			m_EndMip = m_BeginMip + 1;
		}

	CSubresourceSubset::CSubresourceSubset( const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags )
		: m_BeginArray( 0 )
		, m_EndArray( 1 )
		, m_BeginMip( 0 )
		, m_BeginPlane( 0 )
		, m_EndPlane( D3D12GetFormatPlaneCount( Renderer::Get()->GetD3D12Device(), ResourceFormat ) )
		{
			switch ( Desc.ViewDimension )
			{
			default:
					break;

			case ( D3D12_DSV_DIMENSION_TEXTURE1D ):
					m_BeginMip = uint8( Desc.Texture1D.MipSlice );
					break;

			case ( D3D12_DSV_DIMENSION_TEXTURE1DARRAY ):
					m_BeginArray = uint16( Desc.Texture1DArray.FirstArraySlice );
					m_EndArray   = uint16( m_BeginArray + Desc.Texture1DArray.ArraySize );
					m_BeginMip   = uint8( Desc.Texture1DArray.MipSlice );
					break;

			case ( D3D12_DSV_DIMENSION_TEXTURE2D ):
					m_BeginMip = uint8( Desc.Texture2D.MipSlice );
					break;

			case ( D3D12_DSV_DIMENSION_TEXTURE2DMS ):
					break;

			case ( D3D12_DSV_DIMENSION_TEXTURE2DARRAY ):
					m_BeginArray = uint16( Desc.Texture2DArray.FirstArraySlice );
					m_EndArray   = uint16( m_BeginArray + Desc.Texture2DArray.ArraySize );
					m_BeginMip   = uint8( Desc.Texture2DArray.MipSlice );
					break;

			case ( D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY ):
					m_BeginArray = uint16( Desc.Texture2DMSArray.FirstArraySlice );
					m_EndArray   = uint16( m_BeginArray + Desc.Texture2DMSArray.ArraySize );
					break;
			}

			m_EndMip = m_BeginMip + 1;

			if ( m_EndPlane == 2 )
			{
					if ( ( Flags & ViewSubresourceSubsetFlags_DepthAndStencilDsv ) !=
							ViewSubresourceSubsetFlags_DepthAndStencilDsv )
					{
						if ( Flags & ViewSubresourceSubsetFlags_DepthOnlyDsv )
						{
							m_BeginPlane = 0;
							m_EndPlane   = 1;
						}
						else if ( Flags & ViewSubresourceSubsetFlags_StencilOnlyDsv )
						{
							m_BeginPlane = 1;
							m_EndPlane   = 2;
						}
					}
			}
		}

	bool CSubresourceSubset::DoesNotOverlap( const CSubresourceSubset& other ) const
	{
		if ( m_EndArray <= other.m_BeginArray )
		{
			return true;
		}

		if ( other.m_EndArray <= m_BeginArray )
		{
			return true;
		}

		if ( m_EndMip <= other.m_BeginMip )
		{
			return true;
		}

		if ( other.m_EndMip <= m_BeginMip )
		{
			return true;
		}

		if ( m_EndPlane <= other.m_BeginPlane )
		{
			return true;
		}

		if ( other.m_EndPlane <= m_BeginPlane )
		{
			return true;
		}

		return false;
	}

	CViewSubresourceSubset::CViewSubresourceSubset( uint32 Subresource, uint8 MipLevels, uint16 ArraySize, uint8 PlaneCount )
		: m_MipLevels( MipLevels )
		, m_ArraySlices( ArraySize )
		, m_PlaneCount( PlaneCount )
	{
		if ( Subresource < uint32( MipLevels ) * uint32( ArraySize ) )
		{
			m_BeginArray = Subresource / MipLevels;
			m_EndArray   = m_BeginArray + 1;
			m_BeginMip   = Subresource % MipLevels;
			m_EndMip     = m_EndArray + 1;
		}
		else
		{
			m_BeginArray = 0;
			m_BeginMip   = 0;
			if ( Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES )
			{
			m_EndArray = ArraySize;
			m_EndMip   = MipLevels;
			}
			else
			{
			m_EndArray = 0;
			m_EndMip   = 0;
			}
		}
		m_MostDetailedMip = m_BeginMip;
		m_ViewArraySize   = m_EndArray - m_BeginArray;
	}

	CViewSubresourceSubset::CViewSubresourceSubset( const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags)
		: CSubresourceSubset( Desc, ResourceFormat )
		, m_MipLevels( MipLevels )
		, m_ArraySlices( ArraySize )
		, m_PlaneCount( D3D12GetFormatPlaneCount( Renderer::Get()->GetD3D12Device(), ResourceFormat ) )
	{
		if ( Desc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D )
		{
			drn_check( m_BeginArray == 0 );
			m_EndArray = 1;
		}
		m_MostDetailedMip = m_BeginMip;
		m_ViewArraySize   = m_EndArray - m_BeginArray;
		Reduce();
	}

	CViewSubresourceSubset::CViewSubresourceSubset( const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags)
		: CSubresourceSubset( Desc )
		, m_MipLevels( MipLevels )
		, m_ArraySlices( ArraySize )
		, m_PlaneCount( D3D12GetFormatPlaneCount( Renderer::Get()->GetD3D12Device(), ResourceFormat ) )
	{
		if ( Desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D )
		{
			m_BeginArray = 0;
			m_EndArray   = 1;
		}
		m_MostDetailedMip = m_BeginMip;
		m_ViewArraySize   = m_EndArray - m_BeginArray;
		Reduce();
	}



	CViewSubresourceSubset::CViewSubresourceSubset( const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags )
		: CSubresourceSubset( Desc, ResourceFormat, Flags )
		, m_MipLevels( MipLevels )
		, m_ArraySlices( ArraySize )
		, m_PlaneCount( D3D12GetFormatPlaneCount( Renderer::Get()->GetD3D12Device(), ResourceFormat ) )
	{
		m_MostDetailedMip = m_BeginMip;
		m_ViewArraySize   = m_EndArray - m_BeginArray;
		Reduce();
	}

	CViewSubresourceSubset::CViewSubresourceSubset( const D3D12_RENDER_TARGET_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags)
		: CSubresourceSubset( Desc )
		, m_MipLevels( MipLevels )
		, m_ArraySlices( ArraySize )
		, m_PlaneCount( D3D12GetFormatPlaneCount( Renderer::Get()->GetD3D12Device(), ResourceFormat ) )
	{
		if ( Desc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE3D )
		{
			m_BeginArray = 0;
			m_EndArray   = 1;
		}
		m_MostDetailedMip = m_BeginMip;
		m_ViewArraySize   = m_EndArray - m_BeginArray;
		Reduce();
	}

	void CViewSubresourceSubset::Reduce()
	{
		if ( m_BeginMip == 0 && m_EndMip == m_MipLevels && m_BeginArray == 0 && m_EndArray == m_ArraySlices &&
				m_BeginPlane == 0 && m_EndPlane == m_PlaneCount )
		{
			uint32 startSubresource =
				D3D12CalcSubresource( 0, 0, m_BeginPlane, m_MipLevels, m_ArraySlices );
			uint32 endSubresource = D3D12CalcSubresource( 0, 0, m_EndPlane, m_MipLevels, m_ArraySlices );

			// Only coalesce if the full-resolution UINTs fit in the UINT8s used for storage here
			if ( endSubresource < static_cast<uint8>( -1 ) )
			{
			m_BeginArray = 0;
			m_EndArray   = 1;
			m_BeginPlane = 0;
			m_EndPlane   = 1;
			m_BeginMip   = static_cast<uint8>( startSubresource );
			m_EndMip     = static_cast<uint8>( endSubresource );
			}
		}
	}

	uint32 CViewSubresourceSubset::CViewSubresourceIterator::StartSubresource() const
	{
		return D3D12CalcSubresource( m_Subresources.m_BeginMip, m_CurrentArraySlice, m_CurrentPlaneSlice, m_Subresources.m_MipLevels, m_Subresources.m_ArraySlices );
	}

	uint32 CViewSubresourceSubset::CViewSubresourceIterator::EndSubresource() const
	{
		return D3D12CalcSubresource( m_Subresources.m_EndMip, m_CurrentArraySlice, m_CurrentPlaneSlice, m_Subresources.m_MipLevels, m_Subresources.m_ArraySlices );
	}

	std::pair<uint32, uint32> CViewSubresourceSubset::CViewSubresourceIterator::operator*() const
	{
		std::pair<uint32, uint32> NewPair;
		NewPair.first  = StartSubresource();
		NewPair.second = EndSubresource();
		return NewPair;
	}


        }  // namespace Drn