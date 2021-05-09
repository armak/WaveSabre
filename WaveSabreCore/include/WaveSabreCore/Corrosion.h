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
		virtual int GetProcessingDelay() const override;

	private:
		float shape(float input, float p1, float p2);
		void createSincImpulse(float* result, const int taps, const double cutoff);

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

		float previousBuffer[2][65536] = {};
		float oversamplingBuffer[2][65536] = {};
		float waveshapingBuffer[2][65536] = {};
		float bandlimitingBuffer[2][65536] = {};
		
		static const int Taps2 = 64;
		static const int Taps4 = 128;
		float firResponse2[Taps2];
		float firResponse4[Taps4];
	};
}

#endif
