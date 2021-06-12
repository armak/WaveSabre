#ifndef __WAVESABRECORE_MULTIBAND_H__
#define __WAVESABRECORE_MULTIBAND_H__

#include "Device.h"
#include "DelayBuffer.h"
#include "ButterworthFilter.h"

namespace WaveSabreCore
{
	class Multiband : public Device
	{
	public:
		enum class ParamIndices
		{
			Sidechain,
			InputGain,
			
			LowBandCutoff,
			MidBandCutoff,
			HighBandCutoff,

			Band1Threshold,
			Band1Ratio,
			Band1Attack,
			Band1Release,
			Band1Gain,

			Band2Threshold,
			Band2Ratio,
			Band2Attack,
			Band2Release,
			Band2Gain,

			Band3Threshold,
			Band3Ratio,
			Band3Attack,
			Band3Release,
			Band3Gain,

			Band4Threshold,
			Band4Ratio,
			Band4Attack,
			Band4Release,
			Band4Gain,
			
			NumParams,
		};

		Multiband();
		virtual ~Multiband();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		struct LinkwitzRileyCrossover
		{
			ButterworthFilter low[2][2];
			ButterworthFilter high[2][2];
		};

		struct CompressionBand
		{
			LinkwitzRileyCrossover crossover;
			float threshold, ratio;
			float attack, release;
			float gain;
		};

		CompressionBand bands[4];
	};
}

#endif
