#include <WaveSabreCore/Zapper.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Zapper::Zapper() : Device((int)ParamIndices::NumParams)
	{
		for(int i = 0; i < AllpassCount; ++i)
		{
			allpass[0][i].SetType(BiquadFilterType::Allpass);
			allpass[1][i].SetType(BiquadFilterType::Allpass);
		}
	}

	void Zapper::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const int filters = static_cast<int>(static_cast<float>(AllpassCount) * strength + 0.5f);

		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < filters; ++j)
			{
				allpass[i][j].SetFreq(frequency);
				allpass[i][j].SetQ(q);
			}

			for(int j = 0; j < numSamples; ++j)
			{
				float state = allpass[i][0].Next(inputs[i][j]);
				for(int k = 1; k < filters; ++k)
				{
					state = allpass[i][k].Next(state);
				}

				outputs[i][j] = state;
			}
		}
	}

	void Zapper::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Frequency: frequency = Helpers::ParamToFrequency(value); break;
		case ParamIndices::Q: q = Helpers::ParamToAllpassQ(value); break;
		case ParamIndices::Strength: strength = value; break;
		}
	}

	float Zapper::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Frequency:
		default:
			return Helpers::FrequencyToParam(frequency);

		case ParamIndices::Q: return Helpers::AllpassQToParam(q);
		case ParamIndices::Strength: return strength;
		}
	}
}
