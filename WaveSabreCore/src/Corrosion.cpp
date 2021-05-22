#include <WaveSabreCore/Corrosion.h>
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

	// https://stackoverflow.com/questions/13219146/how-to-sum-m256-horizontally
	__inline float horizontalVectorSum(const __m256 x)
	{
		const __m128 hiQuad = _mm256_extractf128_ps(x, 1);
		const __m128 loQuad = _mm256_castps256_ps128(x);
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
						__m256 sample = _mm256_setzero_ps();
						for(int k = 0; k < Taps2; k += 8)
						{
							const __m256 buffer = _mm256_loadu_ps( &(upsamplingBuffer[i][j + k]) );
							const __m256 kernel = _mm256_load_ps( &(firResponse2[k]) );
							sample = _mm256_add_ps(sample, _mm256_mul_ps(buffer, kernel));
						}

						oversampleBuffer[i][j] = 2.0f * horizontalVectorSum(sample);
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
						__m256 sample = _mm256_setzero_ps();
						for(int k = 0; k < Taps4; k += 8)
						{
							const __m256 buffer = _mm256_loadu_ps( &(upsamplingBuffer[i][j + k]) );
							const __m256 kernel = _mm256_load_ps( &(firResponse4[k]) );
							sample = _mm256_add_ps(sample, _mm256_mul_ps(buffer, kernel));
						}

						oversampleBuffer[i][j] = 4.0f * horizontalVectorSum(sample);
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
						__m256 sample = _mm256_setzero_ps();
						for(int k = 0; k < Taps2; k += 8)
						{
							const __m256 buffer = _mm256_loadu_ps( &(oversampleBuffer[i][j + k]) );
							const __m256 kernel = _mm256_load_ps( &(firResponse2[k]) );
							sample = _mm256_add_ps(sample, _mm256_mul_ps(buffer, kernel));
						}

						bandlimitingBuffer[i][j] = horizontalVectorSum(sample);
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
						__m256 sample = _mm256_setzero_ps();
						for(int k = 0; k < Taps4; k += 8)
						{
							const __m256 buffer = _mm256_loadu_ps( &(oversampleBuffer[i][j + k]) );
							const __m256 kernel = _mm256_load_ps( &(firResponse4[k]) );
							sample = _mm256_add_ps(sample, _mm256_mul_ps(buffer, kernel));
						}

						bandlimitingBuffer[i][j] = horizontalVectorSum(sample);
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

	const float Corrosion::TwoPi = 2.0f * static_cast<float>(3.141592653589793);

	Corrosion::Corrosion() :
		Device((int)ParamIndices::NumParams),
		inputGain(0.0f),
		even(0.0f),
		twist(0.0f),
		fold(0.0f),
		saturation(0.0f),
		clip(0.0f),
		outputGain(0.0f),
		dryWet(1.0f),
		oversampling(OversamplingBuffer::Oversampling::X1),
		dcBlocking(DCBlock::Off),
		buffer(OversamplingBuffer::Oversampling::X1)
	{
	}

	void Corrosion::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		// Note: multiply sample rate with oversampling factor if the behavior ever changes
		// so that the DC filtering is done as part of the waveshaping function, i.e.:
		// float(1 + int(oversampling)*int(oversampling))
		const float R_dc = 1.0f - (TwoPi * 10.0f) / (float(Helpers::CurrentSampleRate));

		const float inputGainScalar  = Helpers::DbToScalar(inputGain);
		const float outputGainScalar = Helpers::DbToScalar(outputGain);
		const float param1 =   4.0f * (even * even);
		const float param2 =  10.0f * (twist * twist);
		const float param3 = 100.0f * (fold * fold);
		const float param4 =  20.0f * saturation;
		const float param5 =  clip;

		// TODO: Everything will fail horribly if numSamples is less than width of the FIR kernel,
		// i.e. ASIO latency lower than 64 samples for 2x, or lower than 128 samples for 4x.

		switch(oversampling)
		{
			case OversamplingBuffer::Oversampling::X1:
			{
				for(int i = 0; i < 2; ++i)
				{
					for(int j = 0; j < numSamples; ++j)
					{
						const float input = inputs[i][j];
						const float inputWithGain = input * inputGainScalar;
						float v = shape(inputWithGain, param1, param2, param3, param4, param5);

						// DC offset removal.
						if(dcBlocking == DCBlock::On)
						{
							const float newPreviousDC = v;
							v = v - previousSampleDC[i] + R_dc * previousSampleNoDC[i];
							previousSampleDC[i] = newPreviousDC;
							previousSampleNoDC[i] = v;
						}

						outputs[i][j] = Helpers::Mix(input, v*outputGainScalar, dryWet);
					}
				}

				break;
			}

			case OversamplingBuffer::Oversampling::X2:
			case OversamplingBuffer::Oversampling::X4:
			{
				buffer.upsampleFrom(inputs, numSamples);

				for(int i = 0; i < 2; ++i)
				{
					const auto length = buffer.getOversampleCount();
					for(int j = 0; j < length; ++j)
					{
						buffer(i, j) = shape(inputGainScalar*buffer(i, j), param1, param2, param3, param4, param5);
					}
				}

				buffer.downsampleTo(outputs);

				// Decimate the bandlimited signal for output.
				for(int i = 0; i < 2; ++i)
				{
					for(int j = 0; j  < numSamples; ++j)
					{
						float v = outputs[i][j];

						// DC offset removal.
						if(dcBlocking == DCBlock::On)
						{
							const float newPreviousDC = v;
							v = v - previousSampleDC[i] + R_dc * previousSampleNoDC[i];
							previousSampleDC[i] = newPreviousDC;
							previousSampleNoDC[i] = v;
						}

						outputs[i][j] = Helpers::Mix(buffer.dry(i, j), v*outputGainScalar, dryWet);
					}
				}

				break;
			}
		}
	}

	float Corrosion::shape(float input, float p1, float p2, float p3, float p4, float p5)
	{
		// Apply even harmonics to the signal.
		float even = input;
		if(p1 > 0.0f)
			even = Helpers::Mix(even,
                                static_cast<float>(Helpers::FastCos(even * Helpers::Mix(1.0f, TwoPi, p1))),
                                Helpers::Clamp(p1, 0.0f, 1.0f)*0.5f);

		// Apply sine function fold wave shaping.
		float twist = even;
		if(p2 > 0.0f)
			twist = Helpers::Mix(even,
                                 static_cast<float>(Helpers::FastSin(even * Helpers::Mix(1.0f, TwoPi, p2))),
                                 Helpers::Clamp(p2, 0.0f, 1.0f));
		
		// Apply foldback distortion.
		float fold = twist;
		if(p3 > 0.0f)
		{
			fold *= (1.0f + p3);
			// TODO: this conditional not strictly needed, maybe faster?
			if(fold > 1.0f || fold < -1.0f)
				fold = fabs(fabs(fmod(fold - 1.0f, 4.0f)) - 2.0f) - 1.0f;
		}

		// Apply odd harmonics saturation using tanh function.
		float tanh = fold;
		if(p4 > 0.0f)
		{
			const float exponent = 2.0f * tanh * (1.0f + p4);

			// Round to one in order to avoid floating point issues in the exponentiation
			// and further calculations below.
			if(exponent >= 16.0f)
			{
				tanh = 1.0f;
			}
			else
			{
				static const float e = 2.71828183f;
				const float exp = Helpers::PowF(e, exponent);
				tanh = (exp - 1.0f) / (exp + 1.0f);
				tanh = Helpers::Mix(fold, tanh, Helpers::Clamp(p4, 0.0f, 1.0f));
			}
		}

		// Hard clipping.
		float clip = tanh;
		if(p5 > 0.0f)
		{
			// Clip to -6dB at maximum setting.
			const float h = Helpers::Mix(1.0f, 0.5f, sqrt(p5));
			const float k = 1.0f + p5*p5*100.0f;
			clip = Helpers::Clamp(clip*k, -h, h);
		}

		return clip;
	}

	void Corrosion::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::InputGain: inputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::Even: even = value; break;
		case ParamIndices::Twist: twist = value; break;
		case ParamIndices::Fold: fold = value; break;
		case ParamIndices::Saturation: saturation = value; break;
		case ParamIndices::Clip: clip = value; break;
		case ParamIndices::OutputGain: outputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::DryWet: dryWet = value; break;
		case ParamIndices::Oversampling:
		{
			oversampling = (OversamplingBuffer::Oversampling)(int)(value * 2.0f);
			buffer.setOversamplingFactor(oversampling);
			break;
		}
		case ParamIndices::DCBlocking: dcBlocking = (DCBlock)(int)(value); break;
		}
	}

	float Corrosion::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::InputGain:
		default:
			return Helpers::DbToParam(inputGain, 12.0f);

		case ParamIndices::Even: return even;
		case ParamIndices::Twist: return twist;
		case ParamIndices::Fold: return fold;
		case ParamIndices::Saturation: return saturation;
		case ParamIndices::Clip: return clip;
		case ParamIndices::OutputGain: return Helpers::DbToParam(outputGain, 12.0f);
		case ParamIndices::DryWet: return dryWet;
		case ParamIndices::Oversampling: return float(oversampling) / 2.0f;
		case ParamIndices::DCBlocking: return float(dcBlocking);
		}
	}

	int Corrosion::GetProcessingDelay() const
	{
		return buffer.getDelaySamples();
	}
}
