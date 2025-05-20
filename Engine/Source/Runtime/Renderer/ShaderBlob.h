#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

namespace Drn
{
	struct ShaderBlob : public Serializable
	{
		ShaderBlob();
		~ShaderBlob();

		ShaderBlob& operator=(ShaderBlob& Other);

		ID3DBlob* m_VS;
		ID3DBlob* m_PS;
		ID3DBlob* m_GS;
		ID3DBlob* m_HS;
		ID3DBlob* m_DS;

		virtual void Serialize(Archive& Ar) override;
		void ReleaseBlobs();
	};
}