#include <WaveSabreCore/ButterworthFilter.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	ButterworthFilter::ButterworthFilter() :
		type(ButterworthFilterType::Lowpass),
		minCutoff(Helpers::CurrentSampleRate * 0.0005f),
		maxCutoff(Helpers::CurrentSampleRate * 0.5f),
		freq(maxCutoff),
		q(0.0f),
		gain(1.0f),
		t0(4.0f * Helpers::CurrentSampleRate * Helpers::CurrentSampleRate),
		t1(8.0f * Helpers::CurrentSampleRate * Helpers::CurrentSampleRate),
		t2(2.0f * Helpers::CurrentSampleRate),
		t3(3.1415926f / Helpers::CurrentSampleRate),
		c0(0.0f), c1(0.0f), c2(0.0f), c3(0.0f),
		hist1(0.0f), hist2(0.0f), hist3(0.0f), hist4(0.0f)
	{
	}

	void ButterworthFilter::SetType(ButterworthFilterType type)
	{
	}

	void ButterworthFilter::Set(float freq, float q)
	{
		freq = Helpers::Clamp(freq, minCutoff, maxCutoff);
		q = Helpers::Clamp(q, 0.0f, 1.0f);

		float wp = t2 * tanf(t3 * freq);
		static const float BUDDA_Q_SCALE = 6.0f;
		q *= BUDDA_Q_SCALE;
		q += 1.0f;

		float b1 = (0.765367f / q) / wp;
		float b2 = 1.0f / (wp * wp);
		float bdTemp = t0 * b2 + 1.0f;
		float bd = 1.0f / (bdTemp + t2 * b1);

		gain = bd;
		c2 = 2.0f - t1 * b2;
		c0 = c2 * bd;
		c1 = (bdTemp - t2 * b1) * bd;

		b1 = (1.847759f / q) / wp;
		bd = 1.0f / (bdTemp + t2 * b1);

		gain *= bd;
		c2 *= bd;
		c3 = (bdTemp - t2 * b1) * bd;
	}

	float ButterworthFilter::Next(float input)
	{
		float output = input * gain;
		float histNext;

		output -= hist1 * c0;
		histNext = output - hist2 * c1;

		output = histNext + hist1 * 2.0f;
		output += hist2;

		hist2 = hist1;
		hist1 = histNext;
		
		output -= hist3 * c2;
		histNext = output - hist4 * c3;

		output = histNext + hist3 * 2.0f;
		output += hist4;

		hist4 = hist3;
		hist3 = histNext;
		
		return output;
	}
}