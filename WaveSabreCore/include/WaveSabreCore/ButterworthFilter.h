#ifndef __WAVESABRECORE_BUTTERWORTHFILTER_H__
#define __WAVESABRECORE_BUTTERWORTHFILTER_H__

namespace WaveSabreCore
{
	enum class ButterworthFilterType
	{
		Lowpass,
		Highpass,
		Bandpass,
		Bandstop
	};

	enum class ButterworthFilterOrder
	{
		Second,
		Fourth,
		Sixth,
		Eighth
	};

	class ButterworthFilter
	{
	public:
		ButterworthFilter();
		ButterworthFilter(ButterworthFilterType filterType);

		float Next(float input);
		void SetType(ButterworthFilterType type);
		void Set(float freq, float q, int order);
		void SetFreq(float freq);

	private:
		ButterworthFilterType type;
		bool recalculate;

		float freq;
		float q;
		float gain;
		float orderFactor;
		int order;

		float a0, a1, a2;
		float b0, b1, b2;

		float z1, z2;
	};
}

#endif
