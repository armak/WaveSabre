#include <WaveSabreCore/OversamplingBuffer.h>
#include <WaveSabreCore/Helpers.h>

#ifdef _DEBUG
#include <cassert>
#include <cmath>
#endif

#include <string.h>

namespace WaveSabreCore
{
	const double OversamplingBuffer::Pi = 3.141592653589793;
	const double OversamplingBuffer::FirCutoffRatio = 21000.0/44100.0;

	float convolveSIMD(const __m256* buffer, const __m256* kernel, const int bufferOffset, const int kernelSize)
	{
		__m256 samples = _mm256_setzero_ps();
		for(int i = 0; i < kernelSize>>3; ++i)
		{
			samples = _mm256_add_ps(samples, _mm256_mul_ps(buffer[i], kernel[i]));
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

	float convolveSIMD2(const float* buffer, const __m256* kernel, const int bufferOffset, const int kernelSize)
	{
		__m256 samples = _mm256_setzero_ps();
		for (int k = 0; k < kernelSize; k += 8)
		{
			const __m256 bufferVector = _mm256_loadu_ps(&(buffer[bufferOffset + k]));
			samples = _mm256_add_ps(samples, _mm256_mul_ps(bufferVector, kernel[k >> 3]));
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

	void OversamplingBuffer::createSincImpulse(__m256* result, const int taps, const double cutoff)
	{
		const double M = static_cast<double>(taps) - 1.0;

		// Generate sinc impulse with a window.
		for(int i = 0; i < taps; i += 8)
		{
			__m256 kernelVector = _mm256_setzero_ps();
			for (int j = 0; j < 8; ++j)
			{
				const double d = static_cast<double>(i + j);
				double impulse = 0.0;
				if (d == M * 0.5)
				{
					impulse = 2.0 * cutoff;
				}
				else
				{
					double n = d - M * 0.5;
					impulse = Helpers::FastSin(2.0 * Pi * cutoff * n) / (Pi * n);
				}

				// Blackman window.
				const double window = 0.42 - (0.5 * Helpers::FastCos(2.0 * Pi * d / M)) + (0.08 * Helpers::FastCos(4.0 * Pi * d / M));
				kernelVector.m256_f32[j] = static_cast<float>(impulse * window);
			}

			result[i>>3] = kernelVector;
		}

#if 0
		float sum = 0.0f;
		for(int i = 0; i < taps; ++i) sum += result[i];
		assert(fabsf(1.0f - sum) < 0.001f);
#endif
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
					reallocateBuffer<float>(dryBuffer, HalfTaps + samples);
					reallocateBuffer<__m256>(upsamplingBuffer, (2 * (Taps2 + samples))>>3);
					reallocateBuffer<float>(oversampleBuffer, 2 * (Taps2 + samples));
					reallocateBuffer<float>(bandlimitingBuffer, 2 * samples);
					oversamplingChanged = false;
				}

				for(int i = 0; i < 2; ++i)
				{
					// Fill scratch buffer for unprocessed signal.
					// We can't just use the input signal directly, because oversampling causes a delay
					// for the processed signal, so we need to delay the dry as well.
					memcpy(dryBuffer[i], previousBuffer[i] + HalfTaps, PreviousCopyBytes);
					memcpy(dryBuffer[i] + HalfTaps, input[i], CurrentCopyBytes);

					for(int j = 0; j < Taps2; j += 4)
					{
						upsamplingBuffer[i][j] = _mm256_setr_ps(previousBuffer[i][j], 0.0f, previousBuffer[i][j + 1], 0.0f, previousBuffer[i][j + 2], 0.0f, previousBuffer[i][j + 3], 0.0f);
					}
					for(int j = 0; j < samples; j += 4)
					{
						upsamplingBuffer[i][j + (Taps2>>2)] = _mm256_setr_ps(input[i][j], 0.0f, input[i][j + 1], 0.0f, input[i][j + 2], 0.0f, input[i][j + 3], 0.0f);
					}

					// Filter above original nyquist and waveshape.
					for(int j = 0; j < samples*2 + Taps2; ++j)
					{
						oversampleBuffer[i][j] = 2.0f * convolveSIMD(upsamplingBuffer[i], firResponse2, j, Taps2);
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
					reallocateBuffer<float>(dryBuffer, HalfTaps + samples);
					reallocateBuffer<__m256>(upsamplingBuffer, (4 * (Taps4 * 2 + samples))>>3);
					reallocateBuffer<float>(oversampleBuffer, 4 * (Taps4 * 2 + samples));
					reallocateBuffer<float>(bandlimitingBuffer, 4 * (Taps4 + samples));
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
					for(int j = 0; j < Taps4; j += 2)
					{
						upsamplingBuffer[i][j] = _mm256_setr_ps(previousBuffer[i][j], 0.0f, 0.0f, 0.0f, previousBuffer[i][j + 1], 0.0f, 0.0f, 0.0f);
					}
					for(int j = 0; j < samples; j += 2)
					{
						upsamplingBuffer[i][j + HalfTaps] = _mm256_setr_ps(input[i][j], 0.0f, 0.0f, 0.0f, input[i][j + 1], 0.0f, 0.0f, 0.0f);
					}

					// Filter above original nyquist and waveshape.
					for(int j = 0; j < samples*4 + Taps4*2; ++j)
					{
						oversampleBuffer[i][j] = 4.0f * convolveSIMD(upsamplingBuffer[i], firResponse4, j, Taps4);
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
						bandlimitingBuffer[i][j] = convolveSIMD2(oversampleBuffer[i], firResponse2, Taps2, j);
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
						bandlimitingBuffer[i][j] = convolveSIMD2(oversampleBuffer[i], firResponse4, Taps4, j);
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
