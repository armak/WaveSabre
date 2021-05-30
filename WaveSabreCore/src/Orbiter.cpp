#include <WaveSabreCore/Orbiter.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Orbiter::Orbiter() : Device((int)ParamIndices::NumParams)
	{
	}

	void Orbiter::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		static const double Nyquist = Helpers::CurrentSampleRate * 0.5;
		static const double OscillatorTrigTerm = 1.0 / Helpers::CurrentSampleRate * 2.0 * M_PI;

		// Map input parameters to better ranges.
		// TODO: could do this only when receiving the updated values.
		const double spreadPhase = spread * Helpers::Mix(M_PI_2, M_PI_4, -1.0f + 2.0f * Helpers::Clamp(rectification, 0.5f, 1.0f));
		const float biasTerm = (-0.5f + bias)*1.25f;
		const float rectifyAmp = 1.0f + rectification;

		const double maxPartial = 3.0 + 49.0 * shape * shape;
		double particlWhole;
		double partialFract = modf(maxPartial, &particlWhole);

		switch(modulator)
		{
			case ModulationSource::Internal:
			{
				for(int i = 0; i < numSamples; ++i)
				{
					// Calculate modulator main fundamental.
					double oscillatorInLeft  = oscillatorPhase[0] * OscillatorTrigTerm;
					double oscillatorInRight = oscillatorPhase[1] * OscillatorTrigTerm;
					float oL = static_cast<float>(Helpers::FastSin(oscillatorInLeft  - spreadPhase));
					float oR = static_cast<float>(Helpers::FastSin(oscillatorInRight + spreadPhase));

					// Shape parameter partials.
					for(double n = 3.0; n < maxPartial && n * frequency < Nyquist; n += 2.0)
					{
						const float partialMask = (n + 1.0 >= maxPartial) ? partialFract : 1.0f;
						oL += static_cast<float>(Helpers::FastSin(n*(oscillatorInLeft  - spreadPhase)) / n) * partialMask;
						oR += static_cast<float>(Helpers::FastSin(n*(oscillatorInRight + spreadPhase)) / n) * partialMask;
					}

					// Perform ring modulation on the input.
					outputs[0][i] = Helpers::Mix(inputs[0][i], inputs[0][i] * rectifyAmp*(Helpers::Mix(oL, fabsf(oL), rectification) + biasTerm), amount);
					outputs[1][i] = Helpers::Mix(inputs[1][i], inputs[1][i] * rectifyAmp*(Helpers::Mix(oR, fabsf(oR), rectification) + biasTerm), amount);

					oscillatorPhase[0] += frequency;
					oscillatorPhase[1] += frequency;
				}

				break;
			}

			case ModulationSource::External:
			{
				for(int i = 0; i < numSamples; ++i)
				{
					// Sidechain modulation source
					const float oL = inputs[2][i];
					const float oR = inputs[3][i];

					// Perform ring modulation on the input.
					outputs[0][i] = Helpers::Mix(inputs[0][i], inputs[0][i] * rectifyAmp*(Helpers::Mix(oL, fabsf(oL), rectification) + biasTerm), amount);
					outputs[1][i] = Helpers::Mix(inputs[1][i], inputs[1][i] * rectifyAmp*(Helpers::Mix(oR, fabsf(oR), rectification) + biasTerm), amount);
				}
				break;
			}
		}
	}

	void Orbiter::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Source: modulator = static_cast<ModulationSource>(Helpers::ParamToBoolean(value)); break;
		case ParamIndices::Frequency: frequency = Helpers::ParamToWideFrequency(value); break;
		case ParamIndices::Shape: shape = value; break;
		case ParamIndices::Rectify: rectification = value; break;
		case ParamIndices::Bias: bias = value; break;
		case ParamIndices::Spread: spread = value; break;
		case ParamIndices::Amount: amount = value; break;
		}
	}

	float Orbiter::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Source:
		default:
			return Helpers::BooleanToParam(static_cast<bool>(modulator));

		case ParamIndices::Frequency: return Helpers::WideFrequencyToParam(frequency);
		case ParamIndices::Shape: return shape;
		case ParamIndices::Rectify: return rectification;
		case ParamIndices::Bias: return bias;
		case ParamIndices::Spread: return spread;
		case ParamIndices::Amount: return amount;
		}
	}
}
