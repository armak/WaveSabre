#ifndef __WAVESABRECORE_ORBITER_H__
#define __WAVESABRECORE_ORBITER_H__

#include "Device.h"
#include "OversamplingBuffer.h"

namespace WaveSabreCore
{
	class Orbiter : public Device
	{
	public:
		enum class ParamIndices
		{
			Source,
			Frequency,
			Bias,
			Rectify,
			Spread,
			Amount,
			Oversampling,

			NumParams,
		};

		Orbiter();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;
		virtual int GetProcessingDelay() const override;

	private:
		enum class ModulationSource
		{
			Sine,
			External
		};

		ModulationSource modulationSource = ModulationSource::Sine;
		float frequency = 1000.0f;
		float bias = 0.5f;
		float rectification = 0.0f;
		float spread = 0.0f;
		float amount = 0.5;
		OversamplingBuffer::Oversampling oversampling = OversamplingBuffer::Oversampling::X1;

		OversamplingBuffer buffer = OversamplingBuffer(OversamplingBuffer::Oversampling::X1);

		double oscillatorValue[2] = {};
		double oscillatorPhase[2] = {};
	};
}

#endif
