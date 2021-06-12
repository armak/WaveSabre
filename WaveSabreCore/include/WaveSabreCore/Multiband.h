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
		enum class Band
		{
			Low,
			LowMid,
			HighMid,
			High
		};

		struct LinkwitzRileyCrossover
		{
			ButterworthFilter low[2][2] = {
				ButterworthFilterType::Lowpass,
				ButterworthFilterType::Lowpass,
				ButterworthFilterType::Lowpass,
				ButterworthFilterType::Lowpass
			};
			ButterworthFilter high[2][2] = {
				ButterworthFilterType::Highpass,
				ButterworthFilterType::Highpass,
				ButterworthFilterType::Highpass,
				ButterworthFilterType::Highpass
			};

			void setCutoff(const float freq)
			{
				cutoff = freq;
				for(int i = 0; i < 2; ++i)
					for(int j = 0; j < 2; ++j)
					{
						low[i][j].SetFreq(freq);
						high[i][j].SetFreq(freq);
					}
			}

			float cutoff;
		};

		struct CompressorBand
		{
			float threshold = 0.0f;
			float ratio = 2.0f;
			float attack = 1.0f;
			float release = 100.0f;
			float gain = 0.0f;

			float peak = 0.0f;
		};

		float gainReduction(const Band band, const float sample, const int channel);

		static const int BandCount = 4;

		LinkwitzRileyCrossover crossovers[BandCount - 1];
		CompressorBand bands[BandCount];
		CompressorBand bandsScalar[BandCount];
		float bandPeaks[2][BandCount] = {};
		float inputGain = 0.5f;
		bool sidechain = false;
	};
}

#endif
