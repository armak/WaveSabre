#ifndef __WAVESABRECORE_CORROSION_H__
#define __WAVESABRECORE_CORROSION_H__

#include "Device.h"
#include "OversamplingBuffer.h"

namespace WaveSabreCore
{
	class Corrosion : public Device
	{
	public:
		enum class ParamIndices
		{
			InputGain,
			Even,
			Twist,
			Fold,
			Saturation,
			Clip,
			OutputGain,
			DryWet,
			Oversampling,
			DCBlocking,

			NumParams,
		};

		Corrosion();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;
		virtual int GetProcessingDelay() const override;

	private:
		float shape(float input, float p1, float p2, float p3, float p4, float p5);

		enum class DCBlock
		{
			Off,
			On,
		};

		static const float TwoPi;

		float inputGain;
		float even;
		float twist;
		float fold;
		float saturation;
		float clip;
		float outputGain;
		float dryWet;
		OversamplingBuffer::Oversampling oversampling;
		DCBlock dcBlocking;

		OversamplingBuffer buffer;

		float previousSampleDC[2]   = {};
		float previousSampleNoDC[2] = {};
	};
}

#endif
