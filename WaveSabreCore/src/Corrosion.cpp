#include <WaveSabreCore/Corrosion.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	const float Corrosion::TwoPi = 2.0f * static_cast<float>(M_PI);
	const float Corrosion::RectFact = sqrtf(2.0f/static_cast<float>(M_PI));

	Corrosion::Corrosion() :
		Device((int)ParamIndices::NumParams),
		inputGain(0.0f),
		rectify(0.0f),
		twist(0.0f),
		fold(0.0f),
		saturation(0.0f),
		clipDrive(0.0f),
		clipThreshold(1.0f),
		clipShape(1.0f),
		outputGain(0.0f),
		dryWet(1.0f),
		oversampling(OversamplingBuffer::Oversampling::X1),
		dcBlocking(DCBlock::Off),
		buffer(OversamplingBuffer::Oversampling::X1)
	{
	}

	void Corrosion::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		// DC filter cutoff frequency at -3dB formula:
		// R = 1 - (2Pi * frequency (Hz) / samplerate (Hz))
		const float R_dc = 1.0f - (TwoPi * 10.0f) / (float(Helpers::CurrentSampleRate));

		const float inputGainScalar  = Helpers::DbToScalar(inputGain);
		const float outputGainScalar = Helpers::DbToScalar(outputGain);
		const float param1 =  20.0f * (rectify * rectify);
		const float param2 =  10.0f * (twist * twist);
		const float param3 = 100.0f * (fold * fold);
		const float param4 =  20.0f * (saturation * saturation);
		const float param5 =  50.0f * (clipDrive * clipDrive);
		const float param6 =  47.5f * (clipShape * clipShape) + 2.5f;

		buffer.submitSamples(inputs, numSamples);
		buffer.upsample(numSamples);

		const auto oversampleCount = buffer.getOversampleCount();

		// Perform waveshaping on the oversampled buffer.
		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < oversampleCount; ++j)
			{
				buffer(i, j) = shape(inputGainScalar*buffer(i, j), param1, param2, param3, param4, param5, param6);
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
	}

	static float tanhf(float x, const float k)
	{
		const float exponent = 2.0f * x * (1.0f + k);
		// Round to one in order to avoid floating point issues in the exponentiation
		// and further calculations below.
		if(exponent >= 16.0f)
		{
			x = 1.0f;
		}
		else
		{
			const float exp = Helpers::PowF(M_E, exponent);
			x = (exp - 1.0f) / (exp + 1.0f);
		}

		return x;
	}

	float Corrosion::shape(float input, float p1, float p2, float p3, float p4, float p5, float p6)
	{
		// Apply even harmonics to the signal.
		float rect = input;
		if(p1 > 0.0f)
		{
			// GELU rectification function approximation.
			rect = rect * (1.0f + tanhf(RectFact*(rect + 0.044715f*rect*rect*rect), p1));
			rect = Helpers::Mix(input, rect, Helpers::Clamp(10.0f*p1, 0.0f, 1.0f));
		}

		// Apply sine function fold wave shaping.
		float twist = rect;
		if(p2 > 0.0f)
		{
			twist = static_cast<float>(Helpers::FastSin(twist * Helpers::Mix(1.0f, TwoPi, p2)));
			twist = Helpers::Mix(rect, twist, Helpers::Clamp(20.0*p2, 0.0f, 1.0f));
		}
		
		// Apply foldback distortion.
		float fold = twist;
		if(p3 > 0.0f)
		{
			fold *= (1.0f + p3);
			if(fold > 1.0f || fold < -1.0f)
				fold = fabsf(fabsf(fmodf(fold - 1.0f, 4.0f)) - 2.0f) - 1.0f;
			fold = Helpers::Mix(twist, fold, Helpers::Clamp(10.0*p3, 0.0f, 1.0f));
		}

		// Apply odd harmonics saturation using tanh function.
		float tanh = fold;
		if(p4 > 0.0f)
		{
			tanh = tanhf(tanh, p4);
			tanh = Helpers::Mix(fold, tanh, Helpers::Clamp(2.0f*p4, 0.0f, 1.0f));
		}

		// Variable hard/soft clipping.
		float clip = tanh;
		if(p5 > 0.0f || clipThreshold < 1.0f)
		{
			double x = clip / clipThreshold;
			x *= 1.0 + p5;

			x = x / Helpers::Pow(1.0 + Helpers::Pow(fabs(x), p6), 1.0/p6);

			clip = x*clipThreshold;
		}

		return clip;
	}

	void Corrosion::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::InputGain: inputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::Rectify: rectify = value; break;
		case ParamIndices::Twist: twist = value; break;
		case ParamIndices::Fold: fold = value; break;
		case ParamIndices::Saturation: saturation = value; break;
		case ParamIndices::ClipDrive: clipDrive = value; break;
		case ParamIndices::ClipThreshold: clipThreshold = 0.1f + 0.9f*value; break;
		case ParamIndices::ClipShape: clipShape = value; break;
		case ParamIndices::OutputGain: outputGain = Helpers::ParamToDb(value, 12.0f); break;
		case ParamIndices::DryWet: dryWet = value; break;
		case ParamIndices::Oversampling:
		{
			oversampling = (OversamplingBuffer::Oversampling)(int)(value * 2.0f);
			buffer.setOversampling(oversampling);
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

		case ParamIndices::Rectify: return rectify;
		case ParamIndices::Twist: return twist;
		case ParamIndices::Fold: return fold;
		case ParamIndices::Saturation: return saturation;
		case ParamIndices::ClipDrive: return clipDrive;
		case ParamIndices::ClipThreshold: return (clipThreshold-0.1f)/0.9f;
		case ParamIndices::ClipShape: return clipShape;
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
