#include <WaveSabreCore/Corrosion.h>
#include <WaveSabreCore/Helpers.h>

#ifdef _DEBUG
#include <cassert>
#include <cmath>
#endif

namespace WaveSabreCore
{
	Corrosion::Corrosion() :
		Device((int)ParamIndices::NumParams),
		inputGain(0.0f),
		even(0.0f),
		twist(0.0f),
		fold(0.0f),
		saturation(0.0f),
		oversampling(Oversampling::X1),
		dryWet(1.0f)
	{
		createSincImpulse(firResponse2, Taps2, 0.243);
		createSincImpulse(firResponse4, Taps4, 0.243*0.5);
	}

	void Corrosion::createSincImpulse(float* result, const int taps, const double cutoff)
	{
		const double Pi = 3.141592653589793;
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

	void Corrosion::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		float inputGainScalar = Helpers::DbToScalar(inputGain);
		const float param1 =   4.0f * (even * even);
		const float param2 =  10.0f * (twist * twist);
		const float param3 = 100.0f * (fold * fold);
		const float param4 =  20.0f * saturation;

		if(oversampling == Oversampling::X1)
		{
			for(int i = 0; i < 2; ++i)
			{
				for(int j = 0; j < numSamples; ++j)
				{
					const float input = inputs[i][j];
					const float inputWithGain = input * inputGainScalar;
					float v = shape(inputWithGain, param1, param2, param3, param4);
					outputs[i][j] = Helpers::Mix(input, v, dryWet);
				}
			}

			return;
		}

		// TODO: Everything will fail horribly if numSamples is less than width of the FIR kernel,
		// i.e. ASIO latency lower than 64 samples for 2x, or lower than 128 samples for 4x.

		if(oversampling == Oversampling::X2)
		{
			const int HalfTaps = Taps2>>1;
			for(int i = 0; i < 2; ++i)
			{
				for(int j = 0; j < HalfTaps; ++j)
				{
					dryBuffer[i][j] = previousBuffer[i][j + HalfTaps];
				}
				for(int j = 0; j < Taps2; ++j)
				{
					oversamplingBuffer[i][j*2]   = previousBuffer[i][j];
					oversamplingBuffer[i][j*2+1] = 0.0f;
				}
				for(int j = 0; j < numSamples; ++j)
				{
					dryBuffer[i][HalfTaps + j] = inputs[i][j];

					oversamplingBuffer[i][(Taps2 + j)*2]   = inputs[i][j];
					oversamplingBuffer[i][(Taps2 + j)*2+1] = 0.0f;
				}

				// Filter above original nyquist and waveshape.
				for(int j = 0; j < numSamples*2 + Taps2; ++j)
				{
					float filteredSample = 0.0f;
					for(int k = 0; k < Taps2; ++k)
					{
						float sample = oversamplingBuffer[i][j + k];
						filteredSample += sample * firResponse2[k];
					}

					waveshapingBuffer[i][j] = shape(2.0f*inputGainScalar*filteredSample, param1, param2, param3, param4);
				}

				// Band limit to original nyquist.
				for(int j = 0; j < numSamples*2; ++j)
				{
					float bandlimitedSample = 0.0f;
					for(int k = 0; k < Taps2; ++k)
					{
						float sample = waveshapingBuffer[i][j + k];
						bandlimitedSample += sample * firResponse2[k];
					}

					bandlimitingBuffer[i][j] = bandlimitedSample;
				}

				// Decimate the bandlimited signal for output.
				for(int j = 0; j  < numSamples; ++j)
				{
					outputs[i][j] = Helpers::Mix(dryBuffer[i][j], bandlimitingBuffer[i][j*2], dryWet);
				}

				for(int j = 0; j < Taps2; ++j)
				{
					previousBuffer[i][j] = inputs[i][numSamples - Taps2 + j];
				}
			}
		}
		else if(oversampling == Oversampling::X4)
		{
			const int HalfTaps = Taps4>>1;
			for(int i = 0; i < 2; ++i)
			{
				for(int j = 0; j < HalfTaps; ++j)
				{
					dryBuffer[i][j] = previousBuffer[i][j + HalfTaps];
				}
				for(int j = 0; j < Taps4; ++j)
				{
					oversamplingBuffer[i][j*4]   = previousBuffer[i][j];
					oversamplingBuffer[i][j*4+1] = 0.0f;
					oversamplingBuffer[i][j*4+2] = 0.0f;
					oversamplingBuffer[i][j*4+3] = 0.0f;
				}
				for(int j = 0; j < numSamples; ++j)
				{
					dryBuffer[i][HalfTaps + j] = inputs[i][j];

					oversamplingBuffer[i][(Taps4 + j)*4]   = inputs[i][j];
					oversamplingBuffer[i][(Taps4 + j)*4+1] = 0.0f;
					oversamplingBuffer[i][(Taps4 + j)*4+2] = 0.0f;
					oversamplingBuffer[i][(Taps4 + j)*4+3] = 0.0f;
				}

				// Filter above original nyquist and waveshape.
				for(int j = 0; j < numSamples*4 + Taps4*2; ++j)
				{
					float filteredSample = 0.0f;
					for(int k = 0; k < Taps4; ++k)
					{
						float sample = oversamplingBuffer[i][j + k];
						filteredSample += sample * firResponse4[k];
					}

					waveshapingBuffer[i][j] = shape(4.0f*inputGainScalar*filteredSample, param1, param2, param3, param4);
				}
				
				// Band limit to original nyquist.
				for(int j = 0; j < numSamples*4 + Taps4; ++j)
				{
					float bandlimitedSample = 0.0f;
					for(int k = 0; k < Taps4; ++k)
					{
						float sample = waveshapingBuffer[i][j + k];
						bandlimitedSample += sample * firResponse4[k];
					}

					bandlimitingBuffer[i][j] = bandlimitedSample;
				}
				
				// Decimate the bandlimited signal for output.
				for(int j = 0; j  < numSamples; ++j)
				{
					outputs[i][j] = Helpers::Mix(dryBuffer[i][j], bandlimitingBuffer[i][j*4 + Taps4], dryWet);
				}

				for(int j = 0; j < Taps4; ++j)
				{
					previousBuffer[i][j] = inputs[i][numSamples - Taps4 + j];
				}
			}
		}
	}

	float Corrosion::shape(float input, float p1, float p2, float p3, float p4)
	{
		// Apply even harmonics to the signal.
		float even = input;
		if(p1 > 0.0f)
			even = Helpers::Mix(input,
                                static_cast<float>(Helpers::FastCos(input * Helpers::Mix(1.0f, 2.0f * 3.141592f, p1))),
                                Helpers::Clamp(p1, 0.0f, 1.0f)*0.5f);

		// Apply sine function fold wave shaping.
		float twist = even;
		if(p2 > 0.0f)
			twist = Helpers::Mix(even,
                                 static_cast<float>(Helpers::FastSin(even * Helpers::Mix(1.0f, 2.0f * 3.141592f, p2))),
                                 Helpers::Clamp(p2, 0.0f, 1.0f));
		
		// Apply foldback distortion.
		float fold = twist;
		if(p3 > 0.0f)
		{
			fold *= (1.0f + p3);
			if(fold > 1.0f || fold < -1.0f)
				fold = fabs(fabs(fmod(fold - 1.0f, 4.0f)) - 2.0f) - 1.0f;
		}

		// Apply odd harmonics saturation using tanh function.
		float tanh = fold;
		if(p4 > 0.0f)
		{
			const float e = 2.71828f;
			const float exp = Helpers::PowF(e, 2.0f * tanh * (1.0f + p4));
			tanh = (exp - 1.0f) / (exp + 1.0f);
			tanh = Helpers::Mix(fold, tanh, Helpers::Clamp(p4, 0.0f, 1.0f));
		}

		return tanh;
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
		case ParamIndices::Oversampling: oversampling = (Oversampling)(int)(value * 2.0f); break;
		case ParamIndices::DryWet: dryWet = value; break;
		}
	}

	float Corrosion::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Twist:
		default:
			return twist;

		case ParamIndices::InputGain: return Helpers::DbToParam(inputGain, 12.0f);
		case ParamIndices::Saturation: return saturation;
		case ParamIndices::Fold: return fold;
		case ParamIndices::Even: return even;
		case ParamIndices::Oversampling: return (float)oversampling / 2.0f;
		case ParamIndices::DryWet: return dryWet;
		}
	}

	int Corrosion::GetProcessingDelay() const
	{
		switch(oversampling)
		{
			case Oversampling::X1:
				return 0;
			case Oversampling::X2:
				return Taps2>>1;
			case Oversampling::X4:
				return Taps4>>1;
			default:
				return 0;
		}
	}
}
