#include <WaveSabreCore/Butter.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Butter::Butter()
		: Device((int)ParamIndices::NumParams)
	{
		type = ButterworthFilterType::Lowpass;
		order = ButterworthFilterOrder::Second;
		cutoff = 1000.0f;
		q = 0.0f;
	}

	void Butter::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const int orderOrdinal = 1 + static_cast<int>(order);
		for (int i = 0; i < 2; i++)
		{
			for(int o = 0; o < orderOrdinal; ++o)
			{
				lowpass[i][o].Set(cutoff, q, orderOrdinal);
				lowpass[i][o].SetType(type);
			}

			for(int j = 0; j < numSamples; j++)
			{
				float state = inputs[i][j];

				for(int o = 0; o < orderOrdinal; ++o)
				{
					state = lowpass[i][o].Next(state);
				}

				outputs[i][j] = state;
			}
		}
	}

	void Butter::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Cutoff: cutoff = Helpers::ParamToFrequency(value); break;
			case ParamIndices::Q: q = value; break;
			case ParamIndices::Type: type = static_cast<ButterworthFilterType>(static_cast<int>(3.0f*value)); break;
			case ParamIndices::Order: order = static_cast<ButterworthFilterOrder>(static_cast<int>(3.0f*value)); break;
		}
	}

	float Butter::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Cutoff:
			default:
				return Helpers::FrequencyToParam(cutoff);

			case ParamIndices::Q: return q;
			case ParamIndices::Type: return static_cast<float>(type)/3.0f;
			case ParamIndices::Order: return static_cast<float>(order)/3.0f;
		}
	}
}
