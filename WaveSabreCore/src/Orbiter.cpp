#include <WaveSabreCore/Orbiter.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Orbiter::Orbiter() : Device((int)ParamIndices::NumParams)
	{
	}

	void Orbiter::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		static const double OscillatorTrigTerm = 1.0 / Helpers::CurrentSampleRate * 2.0 * M_PI;
		const double spreadPhase = spread * M_PI_2;
		const float biasTerm = -0.5f + bias;
		const double oversampleFactor = 1.0 / buffer.getOversamplingFactor();

		buffer.submitSamples(inputs, numSamples);
		buffer.upsample(numSamples);

		const auto oversampleCount = buffer.getOversampleCount();

		for(int j = 0; j < oversampleCount; ++j)
		{
			double oscillatorInLeft  = oscillatorPhase[0] * OscillatorTrigTerm;
			double oscillatorInRight = oscillatorPhase[1] * OscillatorTrigTerm;

			oscillatorValue[0] = Helpers::FastCos(oscillatorInLeft - spreadPhase);
			oscillatorValue[1] = Helpers::FastCos(oscillatorInRight + spreadPhase);

			float oL = static_cast<float>(oscillatorValue[0]);
			float oR = static_cast<float>(oscillatorValue[1]);

			//outputs[0][j] = Helpers::Mix(inputs[0][j], inputs[0][j] * Helpers::Mix(oL, fabsf(oL), rectification) + biasTerm, amount);
			//outputs[1][j] = Helpers::Mix(inputs[1][j], inputs[1][j] * Helpers::Mix(oR, fabsf(oR), rectification) + biasTerm, amount);
			buffer(0, j) = Helpers::Mix(buffer(0, j), buffer(0, j) * Helpers::Mix(oL, fabsf(oL), rectification) + biasTerm, amount);
			buffer(1, j) = Helpers::Mix(buffer(1, j), buffer(1, j) * Helpers::Mix(oR, fabsf(oR), rectification) + biasTerm, amount);

			oscillatorPhase[0] += frequency * oversampleFactor;
			oscillatorPhase[1] += frequency * oversampleFactor;
		}

		buffer.downsampleTo(outputs);
	}

	void Orbiter::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Source: modulationSource = static_cast<ModulationSource>(Helpers::ParamToBoolean(value)); break;
		case ParamIndices::Frequency: frequency = Helpers::ParamToWideFrequency(value); break;
		case ParamIndices::Bias: bias = value; break;
		case ParamIndices::Rectify: rectification = value; break;
		case ParamIndices::Spread: spread = value; break;
		case ParamIndices::Amount: amount = value; break;
		case ParamIndices::Oversampling:
		{
			oversampling = (OversamplingBuffer::Oversampling)(int)(value * 2.0f);
			buffer.setOversamplingFactor(oversampling);
			break;
		}
		}
	}

	float Orbiter::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Source:
		default:
			return Helpers::BooleanToParam(static_cast<bool>(modulationSource));

		case ParamIndices::Frequency: return Helpers::WideFrequencyToParam(frequency);
		case ParamIndices::Bias: return bias;
		case ParamIndices::Rectify: return rectification;
		case ParamIndices::Spread: return spread;
		case ParamIndices::Amount: return amount;
		case ParamIndices::Oversampling: return float(oversampling) / 2.0f;
		}
	}

	int Orbiter::GetProcessingDelay() const
	{
		return buffer.getDelaySamples();
	}
}
