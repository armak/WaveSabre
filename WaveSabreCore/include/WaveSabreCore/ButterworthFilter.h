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

		float Next(float input);

		void SetType(ButterworthFilterType type);

		void Set(float freq, float q, int order);

	private:
		ButterworthFilterType type;
		bool recalculate;

		float freq;
		float q;
		float gain;
		float orderFactor;
		int order;

		double a0, a1, a2;
		double b0, b1, b2;

		double lastInput, lastLastInput;
		double lastOutput, lastLastOutput;
	};
}

#endif
