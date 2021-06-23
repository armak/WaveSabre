#include <WaveSabreCore/Multiband.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Multiband::Multiband() : Device((int)ParamIndices::NumParams)
	{
		crossovers[0].setCutoff(120.0f);
		crossovers[1].setCutoff(800.0f);
		crossovers[2].setCutoff(5400.0f);
	}

	Multiband::~Multiband()
	{
	}

	void Multiband::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const float envCoeff = static_cast<float>(1000.0 / Helpers::CurrentSampleRate);

		for(int i = 0; i < BandCount; ++i)
		{
			bandsScalar[i].threshold = Helpers::DbToScalar(bands[i].threshold);
			bandsScalar[i].ratio = bands[i].ratio;
			bandsScalar[i].attack = envCoeff / bands[i].attack;
			bandsScalar[i].release = envCoeff / bands[i].release;
			bandsScalar[i].gain = Helpers::DbToScalar(bands[i].gain);
		}

		for(int i = 0; i < 2; ++i)
		{
			for(int j = 0; j < numSamples; ++j)
			{
				float s = inputs[i][j];
				
				float lowBand   = crossovers[0].low [i][1].Next(crossovers[0].low [i][0].Next(s));
				float loMidBand = crossovers[1].low [i][1].Next(crossovers[1].low [i][0].Next(crossovers[0].high[i][1].Next(crossovers[0].high[i][0].Next(s))));
				float hiMidBand = crossovers[2].low [i][1].Next(crossovers[2].low [i][0].Next(crossovers[1].high[i][1].Next(crossovers[1].high[i][0].Next(s))));
				float highBand  = crossovers[2].high[i][1].Next(crossovers[2].high[i][0].Next(s));

				lowBand   *= gainReduction(Band::Low, lowBand, i);
				loMidBand *= gainReduction(Band::LowMid, loMidBand, i);
				hiMidBand *= gainReduction(Band::HighMid, hiMidBand, i);
				highBand  *= gainReduction(Band::High, highBand, i);

				outputs[i][j] = lowBand + loMidBand + hiMidBand + highBand;
			}
		}
	}

	float Multiband::gainReduction(const Band band, const float input, const int channel)
	{
		const int index = static_cast<int>(band);
		const float threshold = bandsScalar[index].threshold;
		float& peak = bandPeaks[channel][index];

		float inputLevel = fabsf(input);

		if (inputLevel > peak)
		{
			peak += bandsScalar[index].attack;
			if (peak > inputLevel) peak = inputLevel;
		}
		else
		{
			peak -= bandsScalar[index].release;
			if (peak < inputLevel) peak = inputLevel;
		}

		float gainScalar = bandsScalar[index].gain;
		if (peak > threshold) gainScalar *= (threshold + (peak - threshold) / bandsScalar[index].ratio) / peak;

		return gainScalar;
	}

	void Multiband::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
			case Multiband::ParamIndices::Sidechain: sidechain = Helpers::ParamToBoolean(value); break;
			case Multiband::ParamIndices::InputGain: inputGain = Helpers::ParamToDb(value, 12.0f); break;

			case Multiband::ParamIndices::LowBandCutoff: crossovers[0].setCutoff(Helpers::ParamToFrequency(value)); break;
			case Multiband::ParamIndices::MidBandCutoff: crossovers[1].setCutoff(Helpers::ParamToFrequency(value)); break;
			case Multiband::ParamIndices::HighBandCutoff: crossovers[2].setCutoff(Helpers::ParamToFrequency(value)); break;

			case Multiband::ParamIndices::Band1Threshold: bands[0].threshold = Helpers::ParamToDb(value / 2.0f, 48.0f); break;
			case Multiband::ParamIndices::Band1Ratio: bands[0].ratio = value*value*18.0f + 2.0f; break;
			case Multiband::ParamIndices::Band1Attack: bands[0].attack = Helpers::ScalarToEnvValue(value) / 5.0f; break;
			case Multiband::ParamIndices::Band1Release: bands[0].release = Helpers::ScalarToEnvValue(value); break;
			case Multiband::ParamIndices::Band1Gain: bands[0].gain = Helpers::ParamToDb(value, 12.0f); break;

			case Multiband::ParamIndices::Band2Threshold: bands[1].threshold = Helpers::ParamToDb(value / 2.0f, 48.0f); break;
			case Multiband::ParamIndices::Band2Ratio: bands[1].ratio = value*value*18.0f + 2.0f; break;
			case Multiband::ParamIndices::Band2Attack: bands[1].attack = Helpers::ScalarToEnvValue(value) / 5.0f; break;
			case Multiband::ParamIndices::Band2Release: bands[1].release = Helpers::ScalarToEnvValue(value); break;
			case Multiband::ParamIndices::Band2Gain: bands[1].gain = Helpers::ParamToDb(value, 12.0f); break;

			case Multiband::ParamIndices::Band3Threshold: bands[2].threshold = Helpers::ParamToDb(value / 2.0f, 48.0f); break;
			case Multiband::ParamIndices::Band3Ratio: bands[2].ratio = value*value*18.0f + 2.0f; break;
			case Multiband::ParamIndices::Band3Attack: bands[2].attack = Helpers::ScalarToEnvValue(value) / 5.0f; break;
			case Multiband::ParamIndices::Band3Release: bands[2].release = Helpers::ScalarToEnvValue(value); break;
			case Multiband::ParamIndices::Band3Gain: bands[2].gain = Helpers::ParamToDb(value, 12.0f); break;

			case Multiband::ParamIndices::Band4Threshold: bands[3].threshold = Helpers::ParamToDb(value / 2.0f, 48.0f); break;
			case Multiband::ParamIndices::Band4Ratio: bands[3].ratio = value*value*18.0f + 2.0f; break;
			case Multiband::ParamIndices::Band4Attack: bands[3].attack = Helpers::ScalarToEnvValue(value) / 5.0f; break;
			case Multiband::ParamIndices::Band4Release: bands[3].release = Helpers::ScalarToEnvValue(value); break;
			case Multiband::ParamIndices::Band4Gain: bands[3].gain = Helpers::ParamToDb(value, 12.0f); break;
		}
	}

	float Multiband::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Sidechain:
			default:
				return Helpers::BooleanToParam(sidechain);

			case Multiband::ParamIndices::InputGain: return Helpers::DbToParam(inputGain, 12.0f); break;

			case Multiband::ParamIndices::LowBandCutoff: return Helpers::FrequencyToParam(crossovers[0].cutoff); break;
			case Multiband::ParamIndices::MidBandCutoff: return Helpers::FrequencyToParam(crossovers[1].cutoff); break;
			case Multiband::ParamIndices::HighBandCutoff: return Helpers::FrequencyToParam(crossovers[2].cutoff); break;

			case Multiband::ParamIndices::Band1Threshold: return Helpers::DbToParam(bands[0].threshold, 48.0f) * 2.0f; break;
			case Multiband::ParamIndices::Band1Ratio: return sqrtf((bands[0].ratio - 2.0f) / 18.0f); break;
			case Multiband::ParamIndices::Band1Attack: return Helpers::EnvValueToScalar(bands[0].attack*5.0f); break;
			case Multiband::ParamIndices::Band1Release: return Helpers::EnvValueToScalar(bands[0].release); break;
			case Multiband::ParamIndices::Band1Gain: return Helpers::DbToParam(bands[0].gain, 12.0f); break;

			case Multiband::ParamIndices::Band2Threshold: return Helpers::DbToParam(bands[1].threshold, 48.0f) * 2.0f; break;
			case Multiband::ParamIndices::Band2Ratio: return sqrtf((bands[1].ratio - 2.0f) / 18.0f); break;
			case Multiband::ParamIndices::Band2Attack: return Helpers::EnvValueToScalar(bands[1].attack*5.0f); break;
			case Multiband::ParamIndices::Band2Release: return Helpers::EnvValueToScalar(bands[1].release); break;
			case Multiband::ParamIndices::Band2Gain: return Helpers::DbToParam(bands[1].gain, 12.0f); break;

			case Multiband::ParamIndices::Band3Threshold: return Helpers::DbToParam(bands[2].threshold, 48.0f) * 2.0f; break;
			case Multiband::ParamIndices::Band3Ratio: return sqrtf((bands[2].ratio - 2.0f) / 18.0f); break;
			case Multiband::ParamIndices::Band3Attack: return Helpers::EnvValueToScalar(bands[2].attack*5.0f); break;
			case Multiband::ParamIndices::Band3Release: return Helpers::EnvValueToScalar(bands[2].release); break;
			case Multiband::ParamIndices::Band3Gain: return Helpers::DbToParam(bands[2].gain, 12.0f); break;

			case Multiband::ParamIndices::Band4Threshold: return Helpers::DbToParam(bands[3].threshold, 48.0f) * 2.0f; break;
			case Multiband::ParamIndices::Band4Ratio: return sqrtf((bands[3].ratio - 2.0f) / 18.0f); break;
			case Multiband::ParamIndices::Band4Attack: return Helpers::EnvValueToScalar(bands[3].attack*5.0f); break;
			case Multiband::ParamIndices::Band4Release: return Helpers::EnvValueToScalar(bands[3].release); break;
			case Multiband::ParamIndices::Band4Gain: return Helpers::DbToParam(bands[3].gain, 12.0f); break;
		}
		return 0.0f;
	}
}
