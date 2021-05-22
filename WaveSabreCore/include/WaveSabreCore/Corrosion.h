#ifndef __WAVESABRECORE_CORROSION_H__
#define __WAVESABRECORE_CORROSION_H__

#include "Device.h"

namespace WaveSabreCore
{
	class OversamplingBuffer
	{
	public:
		enum class Oversampling
		{
			X1,
			X2,
			X4,
		};

		OversamplingBuffer() = delete;
		OversamplingBuffer(const Oversampling factor);
		virtual ~OversamplingBuffer();
		void setOversamplingFactor(const Oversampling factor);
		int getOversampleCount() const;
		int getDelaySamples() const;
		void upsampleFrom(float** input, int samples);
		void downsampleTo(float** output);

		float& operator()(const size_t channel, const size_t index)
		{
			return oversampleBuffer[channel][index];
		}

		float dry(const size_t channel, const size_t index)
		{
			return dryBuffer[channel][index];
		}

	private:
		void createSincImpulse(float* result, const int taps, const double cutoff);
		void reallocateBuffer(float* target[2], const size_t count);

		static const double Pi;
		static const double FirCutoffRatio;
		static const int Taps2 = 64;
		static const int Taps4 = 128;
		float firResponse2[Taps2];
		float firResponse4[Taps4];

		float* dryBuffer[2] = {nullptr, nullptr};
		float* upsamplingBuffer[2] = {nullptr, nullptr};
		float* oversampleBuffer[2] = {nullptr, nullptr};
		float* bandlimitingBuffer[2] = {nullptr, nullptr};
		
		float previousBuffer[2][Taps4] = {};

		Oversampling oversampling;
		bool oversamplingChanged;
		int lastFrameSize;
	};

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
