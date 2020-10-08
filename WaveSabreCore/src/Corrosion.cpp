#include <WaveSabreCore/Corrosion.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Corrosion::Corrosion()
		: Device((int)ParamIndices::NumParams)
	{
		inputGain = 0.0f;
		twist = 0.0f;
		saturation = 0.0f;
		oversampling = Oversampling::X1;
		dryWet = 1.0f;

		lastSample[0] = lastSample[1] = 0.0f;

		const float nyquist2 = 22050.0f / 2.0f;
		const float Q2 = 0.5f;
		lowpassUpsample2[0].SetFreq(nyquist2);
		lowpassUpsample2[0].SetQ(Q2);
		lowpassUpsample2[1].SetFreq(nyquist2);
		lowpassUpsample2[1].SetQ(Q2);
		lowpassDownsample2[0].SetFreq(nyquist2);
		lowpassDownsample2[0].SetQ(Q2);
		lowpassDownsample2[1].SetFreq(nyquist2);
		lowpassDownsample2[1].SetQ(Q2);

		const float nyquist4 = 22050.0f / 4.0f;
		const float Q4 = 0.55f;
		lowpassUpsample4[0].SetFreq(nyquist4);
		lowpassUpsample4[0].SetQ(Q4);
		lowpassUpsample4[1].SetFreq(nyquist4);
		lowpassUpsample4[1].SetQ(Q4);
		lowpassDownsample4[0].SetFreq(nyquist4);
		lowpassDownsample4[0].SetQ(Q4);
		lowpassDownsample4[1].SetFreq(nyquist4);
		lowpassDownsample4[1].SetQ(Q4);
	}

	void Corrosion::setQs()
	{
		const float Q = Helpers::Mix(1.0f, 0.5f, Helpers::PowF(saturation, 0.5f));
		lowpassUpsample2[0].SetQ(Q);
		lowpassUpsample2[1].SetQ(Q);
		lowpassUpsample2[0].SetQ(Q);
		lowpassUpsample2[1].SetQ(Q);
		lowpassDownsample2[0].SetQ(Q);
		lowpassDownsample2[1].SetQ(Q);
		lowpassDownsample2[0].SetQ(Q);
		lowpassDownsample2[1].SetQ(Q);
	}

	void Corrosion::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const float param1 = 10.0f * (twist * twist);
		const float param2 = 20.0f * saturation;

		setQs();

		float inputGainScalar = Helpers::DbToScalar(inputGain);

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < numSamples; j++)
			{
				const float input = inputs[i][j];
				const float inputWithGain = input * inputGainScalar;
				
				switch (oversampling)
				{
				case Oversampling::X1:
					{
						float v = shape(inputWithGain, param1, param2);
						outputs[i][j] = Helpers::Mix(input, v, dryWet);
						lastSample[i] = inputWithGain;
					}
					break;
				case Oversampling::X2:
					{
						//float inputMid = (lastSample[i] + inputWithGain) * 0.5f;
						//float vMid = shape(inputMid, param1, param2);
						//v = (vMid + v) * 0.5f;

						// Insert zeros and lowpass.
						// Filtered signal requires gain due to the zeros.
#if 1
						buffer[i][j*2]   = lowpassUpsample2[i].Next((lastSample[i]+inputWithGain)*0.5f);
						buffer[i][j*2+1] = lowpassUpsample2[i].Next(inputWithGain);
#else
						buffer[i][j*2]   = 2.0f*lowpassUpsample2[i].Next(inputWithGain);
						buffer[i][j*2+1] = 2.0f*lowpassUpsample2[i].Next(0.0f);
						
#endif
						

						// Oversample waveshaping.
						buffer[i][j*2]   = shape(buffer[i][j*2], param1, param2);
						buffer[i][j*2+1] = shape(buffer[i][j*2+1], param1, param2);

						// Lowpass result.
						buffer[i][j*2]   = lowpassDownsample2[i].Next(buffer[i][j*2]);
						buffer[i][j*2+1] = lowpassDownsample2[i].Next(buffer[i][j*2+1]);

						lastSample[i] = inputWithGain;
					}
					break;

				case Oversampling::X4:
					{
#ifdef CHEAP_OVERSAMPLING
						float v = shape(inputWithGain, param1, param2);
						float inputMid = (lastSample[i] + inputWithGain) * 0.5f;
						float inputQ1 = (lastSample[i] + inputMid) * 0.5f;
						float inputQ2 = (inputMid + inputWithGain) * 0.5f;
						float vQ1 = shape(inputQ1, param1, param2);
						float vMid = shape(inputMid, param1, param2);
						float vQ2 = shape(inputQ2, param1, param2);
						v = (vQ1 + vMid + vQ2 + v) * 0.25f;
						outputs[i][j] = Helpers::Mix(input, v, dryWet);
						lastSample[i] = inputWithGain;
						
#else
						// Insert zeros and lowpass.
						// Filtered signal requires gain due to the zeros.
#if 0
						float inputMid = (lastSample[i]+inputWithGain)*0.5f;
						float inputQ1 = (lastSample[i]+inputMid)*0.5f;
						float inputQ2 = (inputMid+inputWithGain)*0.5f;
						buffer[i][j*4]   = lowpassUpsample4[i].Next(inputQ1);
						buffer[i][j*4+1] = lowpassUpsample4[i].Next(inputMid);
						buffer[i][j*4+2] = lowpassUpsample4[i].Next(inputQ2);
						buffer[i][j*4+3] = lowpassUpsample4[i].Next(inputWithGain);
#else
						buffer[i][j*4]   = 4.0f*lowpassUpsample4[i].Next(inputWithGain);
						buffer[i][j*4+1] = 4.0f*lowpassUpsample4[i].Next(0.0f);
						buffer[i][j*4+2] = 4.0f*lowpassUpsample4[i].Next(0.0f);
						buffer[i][j*4+3] = 4.0f*lowpassUpsample4[i].Next(0.0f);
#endif

						// Oversample waveshaping.
						buffer[i][j*4]   = shape(buffer[i][j*4], param1, param2);
						buffer[i][j*4+1] = shape(buffer[i][j*4+1], param1, param2);
						buffer[i][j*4+2] = shape(buffer[i][j*4+2], param1, param2);
						buffer[i][j*4+3] = shape(buffer[i][j*4+3], param1, param2);

						// Lowpass result.
						buffer[i][j*4]   = lowpassDownsample4[i].Next(buffer[i][j*4]);
						buffer[i][j*4+1] = lowpassDownsample4[i].Next(buffer[i][j*4+1]);
						buffer[i][j*4+2] = lowpassDownsample4[i].Next(buffer[i][j*4+2]);
						buffer[i][j*4+3] = lowpassDownsample4[i].Next(buffer[i][j*4+3]);
#endif
					}
					break;
				}
			}
		}

#ifndef CHEAP_OVERSAMPLING
		// Decimate oversampled result.
		if(oversampling == Oversampling::X2)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < numSamples; j++)
				{
#ifdef AVERAGE
					outputs[i][j] = Helpers::Mix(inputs[i][j], (buffer[i][j*2]+buffer[i][j*2+1])*0.5f, dryWet);
#else
					outputs[i][j] = Helpers::Mix(inputs[i][j], buffer[i][j*2+1], dryWet);
#endif
				}
			}
		}
		else if(oversampling == Oversampling::X4)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < numSamples; j++)
				{
#ifdef AVERAGE
					outputs[i][j] = Helpers::Mix(input, (buffer[i][j*4]+buffer[i][j*4+1]+buffer[i][j*4+2]+buffer[i][j*4+3])*0.25f, dryWet);
#else
					// Don't know which sample is correct to decimate...
					// the third sample seems to have the best characteristics.
					outputs[i][j] = Helpers::Mix(inputs[i][j], buffer[i][j*4+2], dryWet);
#endif
				}
			}
		}
#endif
	}

	float Corrosion::shape(float input, float p1, float p2)
	{
		// Apply sine function wave shaping.
		const float twist = Helpers::Mix(input, Helpers::FastSin(input * Helpers::Mix(1.0f, 2.0f * 3.141592f, p1)), Helpers::Clamp(0.0f, 1.0f, input));
		
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
}
