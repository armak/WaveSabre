#ifndef __WAVESABRECORE_ORBITER_H__
#define __WAVESABRECORE_ORBITER_H__

#include "Device.h"

namespace WaveSabreCore
{
	class Orbiter : public Device
	{
	public:
		enum class ParamIndices
		{
			Source,
			Frequency,
			Shape,
			Rectify,
			Bias,
			Spread,
			Amount,

			NumParams,
		};

		Orbiter();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		enum class ModulationSource
		{
			Internal,
			External
		};

		ModulationSource modulator = ModulationSource::Internal;
		float frequency = 1000.0f;
		float shape = 0.0f;
		float rectification = 0.0f;
		float bias = 0.5f;
		float spread = 0.0f;
		float amount = 0.5;

		double oscillatorPhase[2] = {};
	};
}

#endif
