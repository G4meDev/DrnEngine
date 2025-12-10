#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class DeviceChild
	{
	protected:
		class Device* Parent;

	public:
		DeviceChild(class Device* InParent = nullptr) : Parent(InParent) {}
		inline class Device* GetParentDevice() const { return Parent; }
	};

}