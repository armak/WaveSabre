#include <WaveSabreCore/Butter.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	ButterworthFilter::ButterworthFilter()
	{
		recalculate = true;

		type = ButterworthFilterType::Lowpass;

		freq = 1000.0f;
		q = 1.0f;
		gain = 0.0f;
		order = 1;

		lastInput = lastLastInput = 0.0;
		lastOutput = lastLastOutput = 0.0;
	}

	float ButterworthFilter::Next(float input)
	{
		if (recalculate)
		{
			double w0 = M_PI * freq / Helpers::CurrentSampleRate;

			// Factor of 10 gives a maximum resonance of +18dB.
			double Q = 1.0 + 10.0*q*orderFactor;

			switch (type)
			{
				case ButterworthFilterType::Lowpass:
				{
					const double C = 1.0 / (tan(w0));
					a0 = 1.0 / (1.0 + M_SQRT2*C/Q + C*C);
					a1 = 2.0*a0;
					a2 = a0;
					b0 = 0.0;
					b1 = 2.0*a0*(1.0 - C*C);
					b2 = a0 * (1.0 - M_SQRT2*C/Q + C*C);
					break;
				}

				case ButterworthFilterType::Highpass:
				{
					const double C = tan(w0);
					a0 = 1.0 / (1.0 + M_SQRT2*C/Q + C*C);
					a1 = -2.0*a0;
					a2 = a0;
					b0 = 0.0;
					b1 = 2.0*a0*(C*C - 1.0);
					b2 = a0*(1.0 - M_SQRT2*C/Q + C*C);
					break;
				}

				case ButterworthFilterType::Bandpass:
				{
					const double bw = freq/Q;
					double dc = M_PI*bw / Helpers::CurrentSampleRate;
					if(dc >= 0.99*M_PI_2) dc = 0.99*M_PI_2;

					const double C = 1.0 / tan(dc);
					const double D = 2.0 * Helpers::FastCos(2.0*w0);
					a0 = 1.0 / (1.0 + C);
					a1 = 0.0;
					a2 = -a0;
					b1 = -a0*C*D;
					b2 = a0*(C - 1.0);
					break;
				}
			
				case ButterworthFilterType::Bandstop:
				{
					const double bw = freq/Q;
					double dc = M_PI*bw / Helpers::CurrentSampleRate;
					if(dc >= 0.99*M_PI_2) dc = 0.99*M_PI_2;

					const double C = tan(dc);
					const double D = 2.0 * Helpers::FastCos(2.0*w0);
					a0 = 1.0 / (1.0 + C);
					a1 = -a0*D;
					a2 = a0;
					b1 = -a0*D;
					b2 = a0*(1.0 - C);
					break;
				}
			}

			recalculate = false;
		}

		double output = a0*input + a1*lastInput + a2*lastLastInput - b1*lastOutput - b2*lastLastOutput;

		lastLastInput = lastInput;
		lastInput = input;

		if (fabsf(lastOutput) <= .0000001f) lastOutput = 0.0;

		lastLastOutput = lastOutput;
		lastOutput = output;

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

	Butter::Butter()
		: Device((int)ParamIndices::NumParams)
	{
		type = ButterworthFilterType::Lowpass;
		order = ButterworthFilterOrder::Second;
		cutoff = 1000.0f;
		q = 0.0f;
	}

	void Butter::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const int orderOrdinal = 1 + static_cast<int>(order);
		for (int i = 0; i < 2; i++)
		{
			for(int o = 0; o < orderOrdinal; ++o)
			{
				lowpass[i][o].Set(cutoff, q, orderOrdinal);
				lowpass[i][o].SetType(type);
			}

			for(int j = 0; j < numSamples; j++)
			{
				float state = inputs[i][j];

				for(int o = 0; o < orderOrdinal; ++o)
				{
					state = lowpass[i][o].Next(state);
				}

				outputs[i][j] = state;
			}
		}
	}

	void Butter::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Cutoff: cutoff = Helpers::ParamToFrequency(value); break;
			case ParamIndices::Q: q = value; break;
			case ParamIndices::Type: type = static_cast<ButterworthFilterType>(static_cast<int>(3.0f*value)); break;
			case ParamIndices::Order: order = static_cast<ButterworthFilterOrder>(static_cast<int>(3.0f*value)); break;
		}
	}

	float Butter::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Cutoff:
			default:
				return Helpers::FrequencyToParam(cutoff);

			case ParamIndices::Q: return q;
			case ParamIndices::Type: return static_cast<float>(type)/3.0f;
			case ParamIndices::Order: return static_cast<float>(order)/3.0f;
		}
	}
}
