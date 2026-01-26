#include "DrnPCH.h"
#include "CurveFloat.h"

namespace Drn
{
	void CurveFloat::AddKey(float InTime, float InValue, ECurveInterpMode InInterpMode)
	{
		int32 Index = 0;
		for(; Index < Keys.size() && Keys[Index].Time < InTime; ++Index);

		Keys.insert(Keys.begin() + Index, FloatCurveKey(InTime, InValue));
	}

	float CurveFloat::Eval(float InTime, float InDefaultValue)
	{
		const int32 NumKeys = Keys.size();

		float InterpVal = InDefaultValue;

		if (NumKeys == 0) {}

		else if (NumKeys == 1)
		{
			InterpVal = Keys[0].Value;
		}

		else
		{
			if (InTime < Keys[0].Time)
			{
				InterpVal = Keys[0].Value;
			}

			else if (InTime > Keys[NumKeys-1].Time)
			{
				InterpVal = Keys[NumKeys-1].Value;
			}

			else
			{
				int32 first = 1;
				int32 last = NumKeys - 1;
				int32 count = last - first;

				while (count > 0)
				{
					int32 step = count / 2;
					int32 middle = first + step;

					if (InTime >= Keys[middle].Time)
					{
						first = middle + 1;
						count -= step + 1;
					}
					else
					{
						count = step;
					}
				}

				InterpVal = EvalForTwoKeys(Keys[first - 1], Keys[first], InTime);
			}
		}

		return InterpVal;
	}

	float CurveFloat::EvalForTwoKeys( const FloatCurveKey& Key1, const FloatCurveKey& Key2, const float InTime ) const
	{
		drn_check(Key1.InterpMode == ECurveInterpMode::Constant || Key1.InterpMode == ECurveInterpMode::Linear);

		const float Diff = Key2.Time - Key1.Time;

		if (Diff > 0.f && Key1.InterpMode == ECurveInterpMode::Linear)
		{
			const float Alpha = (InTime - Key1.Time) / Diff;
			const float P0 = Key1.Value;
			const float P3 = Key2.Value;

			return std::lerp(P0, P3, Alpha);
		}
		else
		{
			return Key1.Value;
		}
	}

        }