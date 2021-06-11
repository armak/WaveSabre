#include <WaveSabreCore/Multiband.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Multiband::Multiband() : Device((int)ParamIndices::NumParams)
	{
	}

	Multiband::~Multiband()
	{
	}

	void Multiband::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
	}

	void Multiband::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		}
	}

	float Multiband::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		default:
			break;
		}
		return 0.0f;
	}
}
