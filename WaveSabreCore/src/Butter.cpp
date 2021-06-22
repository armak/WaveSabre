#include <WaveSabreCore/Butter.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Butter::Butter() : Device((int)ParamIndices::NumParams)
	{
		for(int i = 0; i < 4; ++i)
		{
			filterLP[0][i].SetType(BiquadFilterType::ButterworthLowpass);
			filterLP[1][i].SetType(BiquadFilterType::ButterworthLowpass);
			filterHP[0][i].SetType(BiquadFilterType::ButterworthHighpass);
			filterHP[1][i].SetType(BiquadFilterType::ButterworthHighpass);
		}

		// TODO: fold these eventually.
		// 2nd order.
		cascadeQTable[0][0] = 0.70710678f/0.70710678f;

		// 4nd order.
		cascadeQTable[1][0] = 0.54119610f/0.70710678f;
		cascadeQTable[1][1] = 1.3065630f/0.70710678f;

		// 6nd order.
		cascadeQTable[2][0] = 0.51763809f/0.70710678f;
		cascadeQTable[2][1] = 0.70710678f/0.70710678f;
		cascadeQTable[2][2] = 1.9318517f/0.70710678f;

		// 8th order.
		cascadeQTable[3][0] = 0.50979558f/0.70710678f;
		cascadeQTable[3][1] = 0.60134489f/0.70710678f;
		cascadeQTable[3][2] = 0.89997622f/0.70710678f;
		cascadeQTable[3][3] = 2.5629154f/0.70710678f;
	}

	void Butter::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const int cascadeCount = static_cast<int>(order);
		// TODO: there's probably a more correct way to formulate the cascaded Q... look into it.
		const float qf = 1.0f + 10.0f*q/(Helpers::PowF(static_cast<float>(1.0f + cascadeCount), 1.52f));
		const float cutoffLow  = Helpers::Clamp(cutoff / (1.0f + 20.0f*q*q*q), 5.0f, 22000.0f);
		const float cutoffHigh = Helpers::Clamp(cutoff * (1.0f + 20.0f*q*q*q), 5.0f, 22000.0f);
		
		// TODO: probably clean this up.
		for (int i = 0; i < 2; i++)
		{
			switch(type)
			{
				case ButterworthFilterType::Lowpass:
				{
					for(int c = 0; c <= cascadeCount; ++c)
					{
						filterLP[i][c].SetFreq(cutoff);
						filterLP[i][c].SetQ(cascadeQTable[cascadeCount][c]*qf);
					}
					for(int j = 0; j < numSamples; j++)
					{
						float state  = inputs[i][j];
						for(int c = 0; c <= cascadeCount; ++c)
						{
							state = filterLP[i][c].Next(state);
						}
						outputs[i][j] = state;
					}
					break;
				}

				case ButterworthFilterType::Highpass:
				{
					for(int c = 0; c <= cascadeCount; ++c)
					{
						filterHP[i][c].SetFreq(cutoff);
						filterHP[i][c].SetQ(cascadeQTable[cascadeCount][c]*qf);
					}
					for(int j = 0; j < numSamples; j++)
					{
						float state  = inputs[i][j];
						for(int c = 0; c <= cascadeCount; ++c)
						{
							state = filterHP[i][c].Next(state);
						}
						outputs[i][j] = state;
					}
					break;
				}

				case ButterworthFilterType::Bandpass:
				{
					for(int c = 0; c <= cascadeCount; ++c)
					{
						filterLP[i][c].SetFreq(cutoffHigh);
						filterLP[i][c].SetQ(cascadeQTable[cascadeCount][c]);
						filterHP[i][c].SetFreq(cutoffLow);
						filterHP[i][c].SetQ(cascadeQTable[cascadeCount][c]);
					}

					for(int j = 0; j < numSamples; j++)
					{
						float state  = inputs[i][j];
						for(int c = 0; c <= cascadeCount; ++c)
						{
							state = filterHP[i][c].Next(filterLP[i][c].Next(state));
						}

						outputs[i][j] = state;
					}
					break;
				}

				case ButterworthFilterType::Bandstop:
				{
					for(int c = 0; c <= cascadeCount; ++c)
					{
						filterLP[i][c].SetFreq(cutoffLow);
						filterLP[i][c].SetQ(cascadeQTable[cascadeCount][c]);
						filterHP[i][c].SetFreq(cutoffHigh);
						filterHP[i][c].SetQ(cascadeQTable[cascadeCount][c]);
					}

					for(int j = 0; j < numSamples; j++)
					{
						float stateLow  = inputs[i][j];
						float stateHigh = stateLow;
						for(int c = 0; c <= cascadeCount; ++c)
						{
							stateLow = filterLP[i][c].Next(stateLow);
							stateHigh = filterHP[i][c].Next(stateHigh);
						}

						outputs[i][j] = stateLow + stateHigh;
					}
					break;
				}
			}
		}
	}

	void Butter::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
			case ParamIndices::Cutoff: cutoff = Helpers::ParamToFrequency(value); break;
			case ParamIndices::Q: q = value; break;
			case ParamIndices::Type: type = static_cast<ButterworthFilterType>(3.0f*value); break;
			case ParamIndices::Order: order = static_cast<ButterworthFilterOrder>(3.0f*value); break;
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
