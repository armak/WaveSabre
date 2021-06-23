#include <WaveSabreCore/ButterworthFilter.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	ButterworthFilter::ButterworthFilter()
	{
		recalculate = true;

		type = ButterworthFilterType::Lowpass;

		freq = 1000.0f;
		q = 0.0f;
		gain = 0.0f;
		order = 1;

		z1 = z2 = 0.0f;
	}

	ButterworthFilter::ButterworthFilter(ButterworthFilterType filterType) : ButterworthFilter()
	{
		type = filterType;
	}

	float ButterworthFilter::Next(float input)
	{
		if (recalculate)
		{
			float w0 = M_PI * freq / Helpers::CurrentSampleRate;

			// Factor of 10 gives a maximum resonance of +18dB.
			float Q = 1.0;//1.0f + 10.0f*q*orderFactor;

			switch (type)
			{
				case ButterworthFilterType::Lowpass:
				{
					const float C = 1.0f / (tanf(w0));
					a0 = 1.0f / (1.0f + M_SQRT2*C/Q + C*C);
					a1 = 2.0f*a0;
					a2 = a0;
					b0 = 0.0f;
					b1 = 2.0f*a0*(1.0f - C*C);
					b2 = a0 * (1.0f - M_SQRT2*C/Q + C*C);
					break;
				}

				case ButterworthFilterType::Highpass:
				{
					const float C = tanf(w0);
					a0 = 1.0f / (1.0f + M_SQRT2*C/Q + C*C);
					a1 = -2.0f*a0;
					a2 = a0;
					b0 = 0.0f;
					b1 = 2.0f*a0*(C*C - 1.0f);
					b2 = a0*(1.0f - M_SQRT2*C/Q + C*C);
					break;
				}

				case ButterworthFilterType::Bandpass:
				{
					float dc = w0/Q;
					if(dc >= 0.99*M_PI_2) dc = 0.99*M_PI_2;

					const float C = 1.0f / tanf(dc);
					const float D = 2.0f * static_cast<float>(Helpers::FastCos(2.0f*w0));
					a0 = 1.0f / (1.0f + C);
					a1 = 0.0f;
					a2 = -a0;
					b1 = -a0*C*D;
					b2 = a0*(C - 1.0f);
					break;
				}
			
				case ButterworthFilterType::Bandstop:
				{
					float dc = w0/Q;
					if(dc >= 0.99f*M_PI_2) dc = 0.99f*M_PI_2;

					const float C = tanf(dc);
					const float D = 2.0f * static_cast<float>(Helpers::FastCos(2.0f*w0));
					a0 = 1.0f / (1.0f + C);
					a1 = -a0*D;
					a2 = a0;
					b1 = -a0*D;
					b2 = a0*(1.0f - C);
					break;
				}
			}

			recalculate = false;
		}

		float output = a0 * input + z1;
		z1 = a1*input - b1*output + z2;
		z2 = a2*input - b2*output;

		return output;
	}

	void ButterworthFilter::SetType(ButterworthFilterType type)
	{
		if (type == this->type)
			return;

		this->type = type;
		recalculate = true;
	}

	void ButterworthFilter::Set(float freq, float q, int order)
	{
		if (freq == this->freq && q == this->q && order == this->order)
			return;

		this->freq = freq;
		this->q = q;
		this->order = order;
		orderFactor = 1.0f / powf(static_cast<float>(order), 1.5f);
		recalculate = true;
	}

	void ButterworthFilter::SetFreq(float freq)
	{
		if (freq == this->freq)
			return;

		this->freq = freq;
		recalculate = true;
	}
}
