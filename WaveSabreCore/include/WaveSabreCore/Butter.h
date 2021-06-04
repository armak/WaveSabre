#ifndef __WAVESABRECORE_BUTTER_H__
#define __WAVESABRECORE_BUTTER_H__

#include "Device.h"
//#include "ButterworthFilter.h"

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

	class Butter : public Device
	{
	public:
		enum class ParamIndices
		{
			Type,
			Order,
			Cutoff,
			Q,

			NumParams,
		};

		Butter();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		float cutoff, q;
		ButterworthFilterType type;
		ButterworthFilterOrder order;
		ButterworthFilter lowpass[2][4];
	};
}

#endif
