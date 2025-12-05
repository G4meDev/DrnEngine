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


}  // namespace Drn