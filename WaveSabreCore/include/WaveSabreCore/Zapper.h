#ifndef __WAVESABRECORE_ZAPPER_H__
#define __WAVESABRECORE_ZAPPER_H__

#include "Device.h"

namespace WaveSabreCore
{
	class Zapper : public Device
	{
	public:
		enum class ParamIndices
		{
			Frequency,
			Q,
			Intensity,

			NumParams,
		};

		Zapper();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		float frequency = 500.0f;
		float q = 0.5f;
		float intensity = 0.5f;
	};
}

#endif
