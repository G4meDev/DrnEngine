#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Serializable
	{
	public:

		virtual void Serialize(Archive& Ar) = 0;

	protected:

	private:
	};
}