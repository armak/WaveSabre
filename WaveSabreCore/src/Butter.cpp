#include <WaveSabreCore/Butter.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Butter::Butter()
		: Device((int)ParamIndices::NumParams)
	{
		cutoff = 20000.0f;
		q = 0.0f;
	}

	void Butter::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		for (int i = 0; i < 2; i++)
		{
			lowpass[i].Set(cutoff, q);

			for (int j = 0; j < numSamples; j++)
			{
				float input = inputs[i][j];
				outputs[i][j] = lowpass[i].Next(input);
			}
		}
	}

	void Butter::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Cutoff: cutoff = Helpers::ParamToFrequency(value); break;
		case ParamIndices::Q: q = value; break;
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
		}
	}
}
