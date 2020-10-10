#ifndef __WAVESABRECORE_CORROSION_H__
#define __WAVESABRECORE_CORROSION_H__

#include "Device.h"
#include "ButterworthFilter.h"
//#include "BiquadFilter.h"

namespace WaveSabreCore
{
	class Corrosion : public Device
	{
	public:
		enum class ParamIndices
		{
			InputGain,
			Twist,
			Saturation,
			Oversampling,
			DryWet,

			NumParams,
		};

		Corrosion();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		float shape(float input, float p1, float p2);

		enum class Oversampling
		{
			X1,
			X2,
			X4,
		};

		float inputGain;
		float twist;
		float saturation;
		Oversampling oversampling;
		float dryWet;

		float buffer[2][65536] = {};

		ButterworthFilter lowpassUpsample2[2];
		ButterworthFilter lowpassDownsample2[2];
		ButterworthFilter lowpassUpsample4[2];
		ButterworthFilter lowpassDownsample4[2];
	};
}

#endif
