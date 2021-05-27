#include <WaveSabreCore/OversamplingBuffer.h>
#include <WaveSabreCore/Helpers.h>

#ifdef _DEBUG
#include <cassert>
#include <cmath>
#endif

#include <string.h>
#include <immintrin.h>

namespace WaveSabreCore
{
	const double OversamplingBuffer::Pi = 3.141592653589793;
	const double OversamplingBuffer::FirCutoffRatio = 21000.0/44100.0;

	float convolveSIMD(const float* buffer, const float* kernel, const int length, const int index)
	{
		__m256 samples = _mm256_setzero_ps();
		for (int k = 0; k < length; k += 8)
		{
			const __m256 bufferVector = _mm256_loadu_ps(&(buffer[index + k]));
			const __m256 kernelVector = _mm256_load_ps(&(kernel[k]));
			samples = _mm256_add_ps(samples, _mm256_mul_ps(bufferVector, kernelVector));
		}

		// https://stackoverflow.com/questions/13219146/how-to-sum-m256-horizontally
		const __m128 hiQuad = _mm256_extractf128_ps(samples, 1);
		const __m128 loQuad = _mm256_castps256_ps128(samples);
		const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
		const __m128 loDual = sumQuad;
		const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
		const __m128 sumDual = _mm_add_ps(loDual, hiDual);
		const __m128 lo = sumDual;
		const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
		const __m128 sum = _mm_add_ss(lo, hi);
		return _mm_cvtss_f32(sum);
	}

	OversamplingBuffer::OversamplingBuffer(const Oversampling factor) : oversampling(factor), oversamplingChanged(true), lastFrameSize(0)
	{
		createSincImpulse(firResponse2, Taps2, FirCutoffRatio * 0.5);
		createSincImpulse(firResponse4, Taps4, FirCutoffRatio * 0.25);
	}

	OversamplingBuffer::~OversamplingBuffer()
	{
		if(dryBuffer[0]) delete[] dryBuffer[0];
		if(dryBuffer[1]) delete[] dryBuffer[1];
		if(upsamplingBuffer[0]) delete[] upsamplingBuffer[0];
		if(upsamplingBuffer[1]) delete[] upsamplingBuffer[1];
		if(oversampleBuffer[0]) delete[] oversampleBuffer[0];
		if(oversampleBuffer[1]) delete[] oversampleBuffer[1];
		if(bandlimitingBuffer[0]) delete[] bandlimitingBuffer[0];
		if(bandlimitingBuffer[1]) delete[] bandlimitingBuffer[1];
	}

	void OversamplingBuffer::createSincImpulse(float* result, const int taps, const double cutoff)
	{
		const double M = static_cast<double>(taps) - 1.0;

		// Generate sinc impulse with a window.
		for(int i = 0; i < taps; ++i)
		{
			const double d = static_cast<double>(i);
			double impulse = 0.0;
			if(d == M*0.5)
			{
				impulse = 2.0*cutoff;
			}
			else
			{
				double n = d - M*0.5;
				impulse = Helpers::FastSin(2.0*Pi*cutoff*n) / (Pi*n);
			}

			// Blackman window.
			const double window = 0.42 - (0.5*Helpers::FastCos(2.0*Pi*d/M)) + (0.08*Helpers::FastCos(4.0*Pi*d/M));
			result[i] = static_cast<float>(impulse * window);
		}

#ifdef _DEBUG
		float sum = 0.0f;
		for(int i = 0; i < taps; ++i) sum += result[i];
		assert(fabsf(1.0f - sum) < 0.001f);
#endif
	}

	void OversamplingBuffer::reallocateBuffer(float* target[2], const size_t count)
	{
		if(target[0]) delete[] target[0];
		if(target[1]) delete[] target[1];
		target[0] = new float[count]();
		target[1] = new float[count]();
	}

	void OversamplingBuffer::setOversamplingFactor(const Oversampling factor)
	{
		oversamplingChanged = (factor != oversampling && !oversamplingChanged);
		oversampling = factor;
	}

	int OversamplingBuffer::getOversampleCount() const
	{
		const auto oversamplingInteger = (int)(oversampling);
		switch(oversampling)
		{
			case Oversampling::X1: return lastFrameSize;
			case Oversampling::X2: return lastFrameSize*2 + Taps2;
			case Oversampling::X4: return lastFrameSize*4 + Taps4*2;
		}
	}

	int OversamplingBuffer::getDelaySamples() const
	{
		switch(oversampling)
		{
			case OversamplingBuffer::Oversampling::X1:
				return 0;
			case OversamplingBuffer::Oversampling::X2:
				return Taps2>>1;
			case OversamplingBuffer::Oversampling::X4:
				return Taps4>>1;
			default:
				return 0;
		}
	}

	void OversamplingBuffer::upsampleFrom(float** input, int samples)
	{
		switch(oversampling)
		{
			case Oversampling::X2:
			{
				const int HalfTaps = Taps2>>1;
				const int PreviousCopyBytes = HalfTaps * sizeof(float);
				const int CurrentCopyBytes = samples * sizeof(float);

				if(lastFrameSize != samples || oversamplingChanged)
				{
					reallocateBuffer(dryBuffer, HalfTaps + samples);
					reallocateBuffer(upsamplingBuffer, 2 * (Taps2 + samples));
					reallocateBuffer(oversampleBuffer, 2 * (Taps2 + samples));
					reallocateBuffer(bandlimitingBuffer, 2 * samples);
					oversamplingChanged = false;
				}

				for(int i = 0; i < 2; ++i)
				{
					// Fill scratch buffer for unprocessed signal.
					// We can't just use the input signal directly, because oversampling causes a delay
					// for the processed signal, so we need to delay the dry as well.
					memcpy(dryBuffer[i], previousBuffer[i] + HalfTaps, PreviousCopyBytes);
					memcpy(dryBuffer[i] + HalfTaps, input[i], CurrentCopyBytes);

					for(int j = 0; j < Taps2; ++j)
					{
						upsamplingBuffer[i][j * 2] = previousBuffer[i][j];
						upsamplingBuffer[i][j * 2 + 1] = 0.0f;
					}
					for(int j = 0; j < samples; ++j)
					{
						const int offset = Taps2 + j;
						upsamplingBuffer[i][offset * 2] = input[i][j];
						upsamplingBuffer[i][offset * 2 + 1] = 0.0f;
					}

					// Filter above original nyquist and waveshape.
					for(int j = 0; j < samples*2 + Taps2; ++j)
					{
						oversampleBuffer[i][j] = 2.0f * convolveSIMD(upsamplingBuffer[i], firResponse2, Taps2, j);
					}

					// The last samples frum current buffer need to be copied for the next round.
					for(int j = 0; j < Taps2; ++j)
					{
						previousBuffer[i][j] = input[i][samples - Taps2 + j];
					}
				}

				break;
			}

			case Oversampling::X4:
			{
				const int HalfTaps = Taps4>>1;
				const int PreviousCopyBytes = HalfTaps * sizeof(float);
				const int CurrentCopyBytes = samples * sizeof(float);

				if(lastFrameSize != samples || oversamplingChanged)
				{
					reallocateBuffer(dryBuffer, HalfTaps + samples);
					reallocateBuffer(upsamplingBuffer, 4 * (Taps4 * 2 + samples));
					reallocateBuffer(oversampleBuffer, 4 * (Taps4 * 2 + samples));
					reallocateBuffer(bandlimitingBuffer, 4 * (Taps4 + samples));
					oversamplingChanged = false;
				}

				for(int i = 0; i < 2; ++i)
				{
					// Fill scratch buffer for unprocessed signal.
					// We can't just use the input signal directly, because oversampling causes a delay
					// for the processed signal, so we need to delay the dry as well.
					memcpy(dryBuffer[i], previousBuffer[i] + HalfTaps, PreviousCopyBytes);
					memcpy(dryBuffer[i] + HalfTaps, input[i], CurrentCopyBytes);

					// Create oversampled source signal with zero stuffing.
					for(int j = 0; j < Taps4; ++j)
					{
						upsamplingBuffer[i][j*4]   = previousBuffer[i][j];
						upsamplingBuffer[i][j*4+1] = 0.0f;
						upsamplingBuffer[i][j*4+2] = 0.0f;
						upsamplingBuffer[i][j*4+3] = 0.0f;
					}
					for(int j = 0; j < samples; ++j)
					{
						const int offset = Taps4 + j;
						upsamplingBuffer[i][offset*4]   = input[i][j];
						upsamplingBuffer[i][offset*4+1] = 0.0f;
						upsamplingBuffer[i][offset*4+2] = 0.0f;
						upsamplingBuffer[i][offset*4+3] = 0.0f;
					}

					// Filter above original nyquist and waveshape.
					for(int j = 0; j < samples*4 + Taps4*2; ++j)
					{
						oversampleBuffer[i][j] = 4.0f * convolveSIMD(upsamplingBuffer[i], firResponse4, Taps4, j);
					}

					// The last samples frum current buffer need to be copied for the next round.
					for(int j = 0; j < Taps4; ++j)
					{
						previousBuffer[i][j] = input[i][samples - Taps4 + j];
					}
				}

				break;
			}
		}
		
		lastFrameSize = samples;
	}

	void OversamplingBuffer::downsampleTo(float** output)
	{
		switch(oversampling)
		{
			case Oversampling::X2:
			{
				for(int i = 0; i < 2; ++i)
				{
					// Band limit to original nyquist.
					for(int j = 0; j < lastFrameSize*2; ++j)
					{
						bandlimitingBuffer[i][j] = convolveSIMD(oversampleBuffer[i], firResponse2, Taps2, j);
					}

					// Decimate the bandlimited signal for output.
					for(int j = 0; j  < lastFrameSize; ++j)
					{
						// Sample with +1 offset to compensate for half a sample delay.
						output[i][j] = bandlimitingBuffer[i][j*2 + 1];
					}
				}

				break;
			}

			case Oversampling::X4:
			{
				for(int i = 0; i < 2; ++i)
				{
					// Band limit to original nyquist.
					for(int j = 0; j < lastFrameSize*4 + Taps4; ++j)
					{
						bandlimitingBuffer[i][j] = convolveSIMD(oversampleBuffer[i], firResponse4, Taps4, j);
					}

					// Decimate the bandlimited signal for output.
					for(int j = 0; j  < lastFrameSize; ++j)
					{
						// Sample with +1 offset to compensate for half a sample delay.
						output[i][j] = bandlimitingBuffer[i][j*4 + Taps4 + 1];
					}
				}

				break;
			}
		}
	}
}
