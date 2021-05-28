#include <WaveSabreCore/Corrosion.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
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
				buffer.submitSamples(inputs, numSamples);
				buffer.upsample(numSamples);

				const auto oversampleCount = buffer.getOversampleCount();

				// Perform waveshaping on the oversampled buffer.
				for(int i = 0; i < 2; ++i)
				{
					for(int j = 0; j < oversampleCount; ++j)
					{
						buffer(i, j) = shape(inputGainScalar*buffer(i, j), param1, param2, param3, param4, param5);
					}
				}

				buffer.downsampleTo(outputs);

				// Perform DC filtering, output gain and dry/wet mixing.
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
