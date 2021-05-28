#ifndef __WAVESABRECORE_OVERSAMPLINGBUFFER_H__
#define __WAVESABRECORE_OVERSAMPLINGBUFFER_H__

#include <immintrin.h>

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
		void createSincImpulse(__m256* result, const int taps, const double cutoff);
		template<typename T>
		void reallocateBuffer(T* target[2], const size_t count)
		{
			if(target[0]) delete[] target[0];
			if(target[1]) delete[] target[1];
			target[0] = new T[count]();
			target[1] = new T[count]();
		}

		static const double Pi;
		static const double FirCutoffRatio;
		static const int Taps2 = 64;
		static const int Taps4 = 128;
		__m256 firResponse2[Taps2>>3];
		__m256 firResponse4[Taps4>>3];

		float* dryBuffer[2] = {nullptr, nullptr};
		__m256* upsamplingBuffer[2] = {nullptr, nullptr};
		float* oversampleBuffer[2] = {nullptr, nullptr};
		float* bandlimitingBuffer[2] = {nullptr, nullptr};
		
		float previousBuffer[2][Taps4] = {};

		Oversampling oversampling;
		bool oversamplingChanged;
		int lastFrameSize;
	};
}

#endif
