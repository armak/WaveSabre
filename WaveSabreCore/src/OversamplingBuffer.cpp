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
	float convolveSIMD(const float* buffer, const float* kernel, const int bufferOffset, const int kernelSize)
	{
		__m256 samples = _mm256_setzero_ps();
		for(int k = 0; k < kernelSize; k += 8)
		{
			const __m256 bufferVector = _mm256_loadu_ps(&(buffer[bufferOffset + k]));
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

	float OversamplingBuffer::RingBuffer::read(const size_t i) const
	{
		return buffer[(i + readPosition) % BufferLength];
	}

	void OversamplingBuffer::RingBuffer::write(const size_t i, const float v)
	{
		buffer[(i + writePosition) % BufferLength] = v;
	}

	void OversamplingBuffer::RingBuffer::incrementReadPosition(const size_t a)
	{
		readPosition = (readPosition + a) % BufferLength;
	}

	void OversamplingBuffer::RingBuffer::incrementWritePosition(const size_t a)
	{
		writePosition = (writePosition + a) % BufferLength;
	}

	const double OversamplingBuffer::Pi = 3.141592653589793;
	const double OversamplingBuffer::FirCutoffRatio = 21000.0/44100.0;

	OversamplingBuffer::OversamplingBuffer(const Oversampling factor) : oversampling(factor), oversamplingChanged(true), lastFrameSize(0)
	{
		createSincImpulse(firResponse2, Taps2, FirCutoffRatio * 0.5);
		createSincImpulse(firResponse4, Taps4, FirCutoffRatio * 0.25);
	}

	OversamplingBuffer::~OversamplingBuffer()
	{
#ifndef STATIC_BUFFERS
		if(dryBuffer[0]) delete[] dryBuffer[0];
		if(dryBuffer[1]) delete[] dryBuffer[1];
		if(upsamplingBuffer[0]) delete[] upsamplingBuffer[0];
		if(upsamplingBuffer[1]) delete[] upsamplingBuffer[1];
		if(oversampleBuffer[0]) delete[] oversampleBuffer[0];
		if(oversampleBuffer[1]) delete[] oversampleBuffer[1];
		if(bandlimitingBuffer[0]) delete[] bandlimitingBuffer[0];
		if(bandlimitingBuffer[1]) delete[] bandlimitingBuffer[1];
#endif
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

	void OversamplingBuffer::submitSamples(float** input, const int sampleCount)
	{
		for(int i = 0; i < sampleCount; ++i)
		{
			inputBuffer[0].write(i, input[0][i]);
			inputBuffer[1].write(i, input[1][i]);
		}
		
		inputBuffer[0].incrementWritePosition(sampleCount);
		inputBuffer[1].incrementWritePosition(sampleCount);
	}

	void OversamplingBuffer::upsample(const int sampleCount)
	{
		switch(oversampling)
		{
			case Oversampling::X2:
			{
				const int HalfTaps = Taps2>>1;
				const int PreviousCopyBytes = HalfTaps * sizeof(float);
				const int CurrentCopyBytes = sampleCount * sizeof(float);

				if(lastFrameSize != sampleCount || oversamplingChanged)
				{
					reallocateBuffer(dryBuffer, HalfTaps + sampleCount);
					reallocateBuffer(upsamplingBuffer, 2 * (Taps2 + sampleCount));
					reallocateBuffer(oversampleBuffer, 2 * (Taps2 + sampleCount));
					reallocateBuffer(bandlimitingBuffer, 2 * sampleCount);
					oversamplingChanged = false;
				}

				for(int i = 0; i < 2; ++i)
				{
					// Fill scratch buffer for unprocessed signal.
					// We can't just use the input signal directly, because oversampling causes a delay
					// for the processed signal, so we need to delay the dry as well.
					memcpy(dryBuffer[i], previousBuffer[i] + HalfTaps, PreviousCopyBytes);
					for(int j = 0; j < sampleCount; ++j)
					{
						(dryBuffer[i][j + HalfTaps]) = inputBuffer[i].read(j);
					}

					for(int j = 0; j < Taps2; ++j)
					{
						upsamplingBuffer[i][j * 2] = previousBuffer[i][j];
						upsamplingBuffer[i][j * 2 + 1] = 0.0f;
					}
					for(int j = 0; j < sampleCount; ++j)
					{
						const int offset = Taps2 + j;
						upsamplingBuffer[i][offset * 2] = inputBuffer[i].read(j);
						upsamplingBuffer[i][offset * 2 + 1] = 0.0f;
					}

					// Filter above original nyquist and waveshape.
					for(int j = 0; j < sampleCount*2 + Taps2; ++j)
					{
						oversampleBuffer[i][j] = 2.0f * convolveSIMD(upsamplingBuffer[i], firResponse2, j, Taps2);
					}

					// The last samples frum current buffer need to be copied for the next round.
					for(int j = 0; j < Taps2; ++j)
					{
						previousBuffer[i][j] = inputBuffer[i].read(sampleCount - Taps2 + j);
					}
				}

				break;
			}

			case Oversampling::X4:
			{
				const int HalfTaps = Taps4>>1;
				const int PreviousCopyBytes = HalfTaps * sizeof(float);
				const int CurrentCopyBytes = sampleCount * sizeof(float);

				if(lastFrameSize != sampleCount || oversamplingChanged)
				{
					reallocateBuffer(dryBuffer, HalfTaps + sampleCount);
					reallocateBuffer(upsamplingBuffer, 4 * (Taps4 * 2 + sampleCount));
					reallocateBuffer(oversampleBuffer, 4 * (Taps4 * 2 + sampleCount));
					reallocateBuffer(bandlimitingBuffer, 4 * (Taps4 + sampleCount));
					oversamplingChanged = false;
				}

				for(int i = 0; i < 2; ++i)
				{
					// Fill scratch buffer for unprocessed signal.
					// We can't just use the input signal directly, because oversampling causes a delay
					// for the processed signal, so we need to delay the dry as well.
					memcpy(dryBuffer[i], previousBuffer[i] + HalfTaps, PreviousCopyBytes);
					for(int j = 0; j < sampleCount; ++j)
					{
						dryBuffer[i][j + HalfTaps] = inputBuffer[i].read(j);
					}

					// Create oversampled source signal with zero stuffing.
					for(int j = 0; j < Taps4; ++j)
					{
						upsamplingBuffer[i][j*4]   = previousBuffer[i][j];
						upsamplingBuffer[i][j*4+1] = 0.0f;
						upsamplingBuffer[i][j*4+2] = 0.0f;
						upsamplingBuffer[i][j*4+3] = 0.0f;
					}
					for(int j = 0; j < sampleCount; ++j)
					{
						const int offset = Taps4 + j;
						upsamplingBuffer[i][offset*4]   = inputBuffer[i].read(j);
						upsamplingBuffer[i][offset*4+1] = 0.0f;
						upsamplingBuffer[i][offset*4+2] = 0.0f;
						upsamplingBuffer[i][offset*4+3] = 0.0f;
					}

					// Filter above original nyquist and waveshape.
					for(int j = 0; j < sampleCount*4 + Taps4*2; ++j)
					{
						oversampleBuffer[i][j] = 4.0f * convolveSIMD(upsamplingBuffer[i], firResponse4, j, Taps4);
					}

					// The last samples frum current buffer need to be copied for the next round.
					for(int j = 0; j < Taps4; ++j)
					{
						previousBuffer[i][j] = inputBuffer[i].read(sampleCount - Taps4 + j);
					}
				}

				break;
			}
		}
		
		lastFrameSize = sampleCount;
		inputBuffer[0].incrementReadPosition(sampleCount);
		inputBuffer[1].incrementReadPosition(sampleCount);
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
						bandlimitingBuffer[i][j] = convolveSIMD(oversampleBuffer[i], firResponse2, j, Taps2);
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
						bandlimitingBuffer[i][j] = convolveSIMD(oversampleBuffer[i], firResponse4, j, Taps4);
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
