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
		twist(0.0f),
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
		for (int i = 0; i < taps; ++i) {
			const double d = static_cast<double>(i);
			double impulse = 0.0;
			if (d == M*0.5) {
				impulse = 2.0*cutoff;
			}
			else {
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
		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < currentBufferLength; ++j)
				pastBuffer[i][j] = currentBuffer[i][j];
			for(int j = 0; j < futureBufferLength; ++j)
				currentBuffer[i][j] = futureBuffer[i][j];
			for(int j = 0; j < numSamples; ++j)
				futureBuffer[i][j] = inputs[i][j];
		}

		pastBufferLength = currentBufferLength;
		currentBufferLength = futureBufferLength;
		futureBufferLength = numSamples;

		if(pastBufferLength < Taps2>>1) return;
		
		float inputGainScalar = Helpers::DbToScalar(inputGain);
		const float param1 = 10.0f * (twist * twist);
		const float param2 = 20.0f * saturation;

		if(oversampling == Oversampling::X1)
		{
			for(int i = 0; i < 2; ++i)
			{
				for(int j = 0; j < currentBufferLength; ++j)
				{
					const float input = currentBuffer[i][j];
					const float inputWithGain = input * inputGainScalar;
					float v = shape(inputWithGain, param1, param2);
					outputs[i][j] = Helpers::Mix(input, v, dryWet);
				}
			}
		}
		else if(oversampling == Oversampling::X2)
		{
#ifdef TEST_FIR
			for(int j = 0; j < pastBufferLength; ++j)
			{
				oversamplingBuffer[i][j]   = pastBuffer[i][j];
			}
			for(int j = 0; j < currentBufferLength; ++j)
			{
				oversamplingBuffer[i][pastBufferLength + j]   = currentBuffer[i][j];
			}
			for(int j = 0; j < futureBufferLength; ++j)
			{
				oversamplingBuffer[i][pastBufferLength+currentBufferLength+j]   = futureBuffer[i][j];
			}

			for(int j = 0; j < currentBufferLength; ++j)
			{
				float filteredSample = 0.0f;
				for(int k = 0; k < Taps; ++k)
				{
					int cs = j + (k - ht);
					float sample = oversamplingBuffer[i][currentBufferLength+cs];
					filteredSample += sample * firResponse[k];
				}

				outputs[i][j] = filteredSample;
			}
#else
			const int HalfTaps = Taps2>>1;
			for(int i = 0; i < 2; ++i)
			{
				// Make a buffer for oversampling, insert zeros between samples.
				for(int j = 0; j < pastBufferLength; ++j)
				{
					oversamplingBuffer[i][j*2]   = pastBuffer[i][j];
					oversamplingBuffer[i][j*2+1] = 0.0f;
				}
				for(int j = 0; j < currentBufferLength; ++j)
				{
					oversamplingBuffer[i][(pastBufferLength + j)*2]   = currentBuffer[i][j];
					oversamplingBuffer[i][(pastBufferLength + j)*2+1] = 0.0f;
				}
				for(int j = 0; j < futureBufferLength; ++j)
				{
					oversamplingBuffer[i][(pastBufferLength+currentBufferLength+j)*2]   = futureBuffer[i][j];
					oversamplingBuffer[i][(pastBufferLength+currentBufferLength+j)*2+1] = 0.0f;
				}
				
				// Filter above original nyquist and waveshape.
				for(int j = -HalfTaps; j < currentBufferLength*2 + HalfTaps; ++j)
				{
					float filteredSample = 0.0f;
					for(int k = 0; k < Taps2; ++k)
					{
						int cs = j + (k - HalfTaps);
						float sample = oversamplingBuffer[i][currentBufferLength*2+cs];
						filteredSample += sample * firResponse2[k];
					}

					waveshapingBuffer[i][j+HalfTaps] = shape(2.0f*inputGainScalar*filteredSample, param1, param2);
				}

				// Band limit to original nyquist
				for(int j = 0; j < currentBufferLength*2; ++j)
				{
					float bandlimitedSample = 0.0f;
					for(int k = 0; k < Taps2; ++k)
					{
						int cs = j + (k - HalfTaps);
						float sample = waveshapingBuffer[i][HalfTaps+cs];
						bandlimitedSample += sample * firResponse2[k];
					}

					bandlimitingBuffer[i][j] = bandlimitedSample;
				}

				// Decimate and hope for the best??
				for(int j = 0; j  < numSamples; ++j)
				{
					outputs[i][j] = Helpers::Mix(currentBuffer[i][j], bandlimitingBuffer[i][j*2], dryWet);
				}
			}
#endif
		}
		else if(oversampling == Oversampling::X4)
		{
			const int HalfTaps = Taps4>>1;
			for(int i = 0; i < 2; ++i)
			{
				// Make a buffer for oversampling, insert zeros between samples.
				for(int j = 0; j < pastBufferLength; ++j)
				{
					oversamplingBuffer[i][j*4]   = pastBuffer[i][j];
					oversamplingBuffer[i][j*4+1] = 0.0f;
					oversamplingBuffer[i][j*4+2] = 0.0f;
					oversamplingBuffer[i][j*4+3] = 0.0f;
				}
				for(int j = 0; j < currentBufferLength; ++j)
				{
					oversamplingBuffer[i][(pastBufferLength + j)*4]   = currentBuffer[i][j];
					oversamplingBuffer[i][(pastBufferLength + j)*4+1] = 0.0f;
					oversamplingBuffer[i][(pastBufferLength + j)*4+2] = 0.0f;
					oversamplingBuffer[i][(pastBufferLength + j)*4+3] = 0.0f;
				}
				for(int j = 0; j < futureBufferLength; ++j)
				{
					oversamplingBuffer[i][(pastBufferLength+currentBufferLength+j)*4]   = futureBuffer[i][j];
					oversamplingBuffer[i][(pastBufferLength+currentBufferLength+j)*4+1] = 0.0f;
					oversamplingBuffer[i][(pastBufferLength+currentBufferLength+j)*4+2] = 0.0f;
					oversamplingBuffer[i][(pastBufferLength+currentBufferLength+j)*4+3] = 0.0f;
				}
				
				// Filter above original nyquist and waveshape.
				for(int j = -HalfTaps; j < currentBufferLength*4 + HalfTaps; ++j)
				{
					float filteredSample = 0.0f;
					for(int k = 0; k < Taps4; ++k)
					{
						int cs = j + (k - HalfTaps);
						float sample = oversamplingBuffer[i][currentBufferLength*4+cs];
						filteredSample += sample * firResponse4[k];
					}

					waveshapingBuffer[i][j+HalfTaps] = shape(4.0f*inputGainScalar*filteredSample, param1, param2);
				}

				// Band limit to original nyquist
				for(int j = 0; j < currentBufferLength*4; ++j)
				{
					float bandlimitedSample = 0.0f;
					for(int k = 0; k < Taps4; ++k)
					{
						int cs = j + (k - HalfTaps);
						float sample = waveshapingBuffer[i][HalfTaps+cs];
						bandlimitedSample += sample * firResponse4[k];
					}

					bandlimitingBuffer[i][j] = bandlimitedSample;
				}

				// Decimate and hope for the best??
				for(int j = 0; j  < numSamples; ++j)
				{
					outputs[i][j] = Helpers::Mix(currentBuffer[i][j], bandlimitingBuffer[i][j*4+3], dryWet);
				}
			}
		}
	}

	float Corrosion::shape(float input, float p1, float p2)
	{
		// Apply sine function wave shaping.
		const float twist = Helpers::Mix(input,
										 static_cast<float>(Helpers::FastSin(input * Helpers::Mix(1.0f, 2.0f * 3.141592f, p1))),
										 Helpers::Clamp(0.0f, 1.0f, p1));
		
		// Apply tanh function wave shaping.
		const float e = 2.71828f;
		const float exp = Helpers::PowF(e, 2.0f * twist * (1.0f + p2));
		const float tanh = (exp - 1.0f) / (exp + 1.0f);
		return Helpers::Mix(twist, tanh, Helpers::Clamp(p2, 0.0f, 1.0f));
	}

	void Corrosion::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::InputGain: inputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::Twist: twist = value; break;
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
