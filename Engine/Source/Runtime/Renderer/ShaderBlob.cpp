#include "DrnPCH.h"
#include "ShaderBlob.h"

namespace Drn
{
	ShaderBlob::ShaderBlob()
		: m_VS(nullptr)
		, m_PS(nullptr)
		, m_GS(nullptr)
		, m_HS(nullptr)
		, m_DS(nullptr)
	{
	}

	ShaderBlob::~ShaderBlob()
	{
		ReleaseBlobs();
	}

	ShaderBlob& ShaderBlob::operator=( ShaderBlob& Other )
	{
		ReleaseBlobs();

		auto CopyBlobConditional = [&]( ID3DBlob*& Source, ID3DBlob*& Dest )
		{
			if (Source)
			{
				D3DCreateBlob( Source->GetBufferSize(), &Dest );
				memcpy( Dest->GetBufferPointer(), Source->GetBufferPointer(), Source->GetBufferSize() );
			}
		};

		CopyBlobConditional( Other.m_VS, m_VS );
		CopyBlobConditional( Other.m_PS, m_PS );
		CopyBlobConditional( Other.m_GS, m_GS );
		CopyBlobConditional( Other.m_HS, m_HS );
		CopyBlobConditional( Other.m_DS, m_DS );

		return *this;
	}

	void ShaderBlob::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_VS >> m_PS >> m_GS >> m_HS >> m_DS;
		}

		else
		{
			Ar << m_VS << m_PS << m_GS << m_HS << m_DS;
		}
	}

	void ShaderBlob::ReleaseBlobs()
	{
		if (m_VS)
		{
			m_VS->Release();
			m_VS = nullptr;
		}

		if (m_PS)
		{
			m_PS->Release();
			m_PS = nullptr;
		}

		if (m_GS)
		{
			m_GS->Release();
			m_GS = nullptr;
		}

		if (m_HS)
		{
			m_HS->Release();
			m_HS = nullptr;
		}

		if (m_DS)
		{
			m_DS->Release();
			m_DS = nullptr;
		}
	}

}
