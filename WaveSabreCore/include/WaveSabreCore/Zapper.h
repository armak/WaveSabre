#ifndef __WAVESABRECORE_ZAPPER_H__
#define __WAVESABRECORE_ZAPPER_H__

#include "Device.h"
#include "BiquadFilter.h"

namespace WaveSabreCore
{
	class Zapper : public Device
	{
	public:
		enum class ParamIndices
		{
			Frequency,
			Q,
			Strength,
			Drywet,

			NumParams,
		};

		Zapper();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		static const int AllpassCount = 100;
		BiquadFilter allpass[2][AllpassCount];

		float frequency = 100.0f;
		float q = 0.1f;
		float strength = 0.25f;
		float drywet = 1.0f;
	};
}

#endif
