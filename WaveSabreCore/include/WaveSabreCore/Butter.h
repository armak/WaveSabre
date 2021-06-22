#ifndef __WAVESABRECORE_BUTTER_H__
#define __WAVESABRECORE_BUTTER_H__

#include "Device.h"
#include "BiquadFilter.h"

namespace WaveSabreCore
{
	class Butter : public Device
	{
	public:
		enum class ParamIndices
		{
			Type,
			Order,
			Cutoff,
			Q,

			NumParams,
		};

		Butter();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		enum class ButterworthFilterType
		{
			Lowpass,
			Highpass,
			Bandpass,
			Bandstop
		};

		enum class ButterworthFilterOrder
		{
			Second,
			Fourth,
			Sixth,
			Eighth
		};

		float cutoff = 1000.0f;
		float q = 0.0f;
		ButterworthFilterType type = ButterworthFilterType::Lowpass;
		ButterworthFilterOrder order = ButterworthFilterOrder::Fourth;

		BiquadFilter filterLP[2][4];
		BiquadFilter filterHP[2][4];
		float cascadeQTable[4][4] = {};
	};
}

#endif
