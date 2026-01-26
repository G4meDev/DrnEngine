#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	enum class ECurveInterpMode : uint8
	{
		Linear,
		Constant,
	};

	struct FloatCurveKey
	{
		float Time;
		float Value;
		ECurveInterpMode InterpMode;

		FloatCurveKey()
			: Time(0.f)
			, Value(0.f)
			, InterpMode(ECurveInterpMode::Linear)
		{ }

		FloatCurveKey(float InTime, float InValue, ECurveInterpMode InInterpMode = ECurveInterpMode::Linear)
			: Time(InTime)
			, Value(InValue)
			, InterpMode(InInterpMode)
		{ }

		//bool Serialize(Archive& Ar);
		//bool operator==(const FloatCurveKey& Other) const;
		//bool operator!=(const FloatCurveKey& Other) const;
		//
		//friend Archive& operator<<(Archive& Ar, FloatCurveKey& P)
		//{
		//	P.Serialize(Ar);
		//	return Ar;
		//}
	};

	struct CurveFloat
	{
		void AddKey(float InTime, float InValue, ECurveInterpMode InInterpMode = ECurveInterpMode::Linear);

		virtual float Eval(float InTime, float InDefaultValue = 0.0f);
		float EvalForTwoKeys(const FloatCurveKey& Key1, const FloatCurveKey& Key2, const float InTime) const;
		virtual void Reset() { Keys.clear(); }
		inline int32 GetNumKeys() const { return Keys.size(); }

		std::vector<FloatCurveKey> Keys;
	};
}