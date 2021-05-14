#ifndef __WAVESABRECORE_CORROSION_H__
#define __WAVESABRECORE_CORROSION_H__

#include "Device.h"

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
		void createSincImpulse(float* result, const int taps, const double cutoff);

		enum class Oversampling
		{
			X1,
			X2,
			X4,
		};

		enum class DCBlock
		{
			Off,
			On,
		};

		static const double Pi;
		static const float TwoPi;

		float inputGain;
		float even;
		float twist;
		float fold;
		float saturation;
		float clip;
		float outputGain;
		float dryWet;
		Oversampling oversampling;
		DCBlock dcBlocking;

		float previousBuffer[2][65536] = {};
		float dryBuffer[2][65536] = {};
		float oversamplingBuffer[2][65536] = {};
		float waveshapingBuffer[2][65536] = {};
		float bandlimitingBuffer[2][65536] = {};
		
		static const int Taps2 = 64;
		static const int Taps4 = 128;
		float firResponse2[Taps2];
		float firResponse4[Taps4];

		float previousSampleDC[2]   = {};
		float previousSampleNoDC[2] = {};
	};
}

#endif
