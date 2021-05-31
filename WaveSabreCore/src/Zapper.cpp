#include <WaveSabreCore/Zapper.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Zapper::Zapper() : Device((int)ParamIndices::NumParams)
	{
	}

	void Zapper::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{

	}

	void Zapper::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Frequency: frequency = Helpers::ParamToFrequency(value); break;
		case ParamIndices::Q: q = value; break;
		case ParamIndices::Intensity: intensity = value; break;
		}
	}

	float Zapper::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Frequency:
		default:
			return frequency;

		case ParamIndices::Q: return q;
		case ParamIndices::Intensity: return intensity;
		}
	}
}