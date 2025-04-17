#include "DrnPCH.h"
#include "Matrix.h"

namespace Drn
{
	Matrix::Matrix( const Transform& InTransform )
	{
		DirectX::XMMATRIX ScaleMat			= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&InTransform.Scale.m_Vector));
		DirectX::XMMATRIX RotationMat		= DirectX::XMMatrixRotationQuaternion(InTransform.Rotation.m_Vector);
		DirectX::XMMATRIX TranslationMat	= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&InTransform.Location.m_Vector));

		m_Matrix = ScaleMat * RotationMat * TranslationMat;
	}

}