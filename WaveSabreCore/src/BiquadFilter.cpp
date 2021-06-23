#include <WaveSabreCore/BiquadFilter.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	BiquadFilter::BiquadFilter()
	{
		recalculate = true;

		type = BiquadFilterType::Lowpass;

		freq = 1000.0f;
		q = 1.0f;
		gain = 0.0f;

		lastInput = lastLastInput = 0.0f;
		lastOutput = lastLastOutput = 0.0f;
	}

	BiquadFilter::BiquadFilter(BiquadFilterType type) : BiquadFilter()
	{
		this->type = type;
	}

	float BiquadFilter::Next(float input)
	{
		if (recalculate)
		{
			float w0 = 2.0f * 3.141592f * freq / (float)Helpers::CurrentSampleRate;

			float alpha = (float)Helpers::FastSin(w0) / (2.0f * q);

			float a0, a1, a2;
			float b0, b1, b2;
			switch (type)
			{
				case BiquadFilterType::Lowpass:
					a0 = 1.0f + alpha;
					a1 = -2.0f * (float)Helpers::FastCos(w0);
					a2 = 1.0f - alpha;
					b0 = (1.0f - (float)Helpers::FastCos(w0)) / 2.0f;
					b1 = 1.0f - (float)Helpers::FastCos(w0);
					b2 = (1.0f - (float)Helpers::FastCos(w0)) / 2.0f;
					break;

				case BiquadFilterType::Highpass:
					a0 = 1.0f + alpha;
					a1 = -2.0f * (float)Helpers::FastCos(w0);
					a2 = 1.0f - alpha;
					b0 = (1.0f + (float)Helpers::FastCos(w0)) / 2.0f;
					b1 = -(1.0f + (float)Helpers::FastCos(w0));
					b2 = (1.0f + (float)Helpers::FastCos(w0)) / 2.0f;
					break;

				case BiquadFilterType::Peak:
				{
					float A = Helpers::Exp10F(gain / 40.0f);
					a0 = 1.0f + alpha / A;
					a1 = -2.0f * (float)Helpers::FastCos(w0);
					a2 = 1.0f - alpha / A;
					b0 = 1.0f + alpha * A;
					b1 = -2.0f * (float)Helpers::FastCos(w0);
					b2 = 1.0f - alpha * A;
					break;
				}

				case BiquadFilterType::Allpass:
					a0 = 1.0f + alpha;
					a1 = -2.0f * (float)Helpers::FastCos(w0);
					a2 = 1.0f - alpha;
					b0 = 1.0f - alpha;
					b1 = -2.0f * (float)Helpers::FastCos(w0);
					b2 = 1.0f + alpha;
					break;

				case BiquadFilterType::ButterworthLowpass:
				{
					const float C = 1.0f / tanf(w0*0.5f);
					a0 = 1.0f / (1.0f + 1.4142136f*C/q + C*C);
					a1 = 2.0f*a0;
					a2 = a0;
					b1 = 2.0f*a0*(1.0f - C*C);
					b2 = a0 * (1.0f - 1.4142136f*C/q + C*C);
					break;
				}

				case BiquadFilterType::ButterworthHighpass:
				{
					const float C = tanf(w0*0.5f);
					a0 = 1.0f / (1.0f + 1.4142136f*C/q + C*C);
					a1 = -2.0f*a0;
					a2 = a0;
					b1 = 2.0f*a0*(C*C - 1.0f);
					b2 = a0*(1.0f - 1.4142136f*C/q + C*C);
					break;
				}
			}

			if(type < BiquadFilterType::ButterworthLowpass)
			{
				c1 = b0 / a0;
				c2 = b1 / a0;
				c3 = b2 / a0;
				c4 = a1 / a0;
				c5 = a2 / a0;
			}
			else
			{
				c1 = a0;
				c2 = a1;
				c3 = a2;
				c4 = b1;
				c5 = b2;
			}

			recalculate = false;
		}

		float output = c1*input + c2*lastInput + c3*lastLastInput - c4*lastOutput - c5*lastLastOutput;

		lastLastInput = lastInput;
		lastInput = input;
		lastLastOutput = lastOutput;
		lastOutput = output;

		return output;
	}

	void BiquadFilter::SetType(BiquadFilterType type)
	{
		if (type == this->type)
			return;

		this->type = type;
		recalculate = true;
	}

	void BiquadFilter::SetFreq(float freq)
	{
		if (freq == this->freq)
			return;

		this->freq = freq;
		recalculate = true;
	}

	void BiquadFilter::SetQ(float q)
	{
		if (q == this->q)
			return;

		this->q = q;
		recalculate = true;
	}

	void BiquadFilter::SetGain(float gain)
	{
		if (gain == this->gain)
			return;

		this->gain = gain;
		recalculate = true;
	}

	void BiquadFilter::CopyCoefficients(const BiquadFilter& source)
	{
		c1 = source.c1;
		c2 = source.c2;
		c3 = source.c3;
		c4 = source.c4;
		c5 = source.c5;
	}
}
