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
