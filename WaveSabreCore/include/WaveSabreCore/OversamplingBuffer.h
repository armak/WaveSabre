#ifndef __WAVESABRECORE_OVERSAMPLINGBUFFER_H__
#define __WAVESABRECORE_OVERSAMPLINGBUFFER_H__

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
}

#endif
