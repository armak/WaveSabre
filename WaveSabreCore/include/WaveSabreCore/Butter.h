#ifndef __WAVESABRECORE_BUTTER_H__
#define __WAVESABRECORE_BUTTER_H__

#include "Device.h"
#include "ButterworthFilter.h"

namespace WaveSabreCore
{
	class Butter : public Device
	{
	public:
		enum class ParamIndices
		{
			Cutoff,
			Q,

			NumParams,
		};

		Butter();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		float cutoff, q;
		ButterworthFilter lowpass[2];
	};
}

#endif
