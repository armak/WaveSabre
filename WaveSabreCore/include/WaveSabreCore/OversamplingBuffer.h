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

		void submitSamples(float** input, const int sampleCount);
		void upsample(const int sampleCount);
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

		class RingBuffer
		{
		public:
			float read(const size_t i) const;
			void write(const size_t i, const float v);
			void incrementReadOffset(const size_t a);
			void incrementWriteOffset(const size_t a);

		private:
			static const int BufferLength = 65536;
			static const int BufferMask = BufferLength - 1;
			float buffer[BufferLength] = {};
			size_t readOffset = 0;
			size_t writeOffset = 0;
		};

		static const double Pi;
		static const double FirCutoffRatio;
		static const int Taps2 = 64;
		static const int Taps4 = 128;
		float firResponse2[Taps2];
		float firResponse4[Taps4];

		RingBuffer inputBuffer[2];

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
