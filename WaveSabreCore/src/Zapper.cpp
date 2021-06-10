#include <WaveSabreCore/Zapper.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	const float Zapper::ChaseFactor = 0.0005f;

	Zapper::Zapper() : Device((int)ParamIndices::NumParams)
	{
		for(int i = 0; i < AllpassCount; ++i)
		{
			allpass[0][i].SetType(BiquadFilterType::Allpass);
			allpass[1][i].SetType(BiquadFilterType::Allpass);
		}
	}

	void Zapper::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const int filters = static_cast<int>(static_cast<float>(AllpassCount) * strength + 0.5f);
		
		static const float MaxFeedback = 0.97f;
		static const float Third = 1.0f/3.0f;
		const float feedbackPolar = -1.0f + 2.0f*feedback;
		const float feedbackSign = feedbackPolar < 0.0f ? -1.0f : 1.0f;
		const float feedbackScalar = MaxFeedback*fabsf(feedbackPolar);
		const float feedbackAmp = Helpers::Mix(1.0f, Third, Helpers::Pow2F(fabsf(feedbackPolar))) / (1.0f-feedbackScalar);

		for(int i = 0; i < numSamples; ++i)
		{
			frequencyChase = Helpers::Mix(frequencyChase, frequency, ChaseFactor);
			qChase         = Helpers::Mix(qChase, q, ChaseFactor);

			allpass[0][0].SetFreq(frequencyChase);
			allpass[0][0].SetQ(qChase);
			allpass[1][0].SetFreq(frequencyChase);
			allpass[1][0].SetQ(qChase);

			const float dryLeft  = inputs[0][i];
			const float dryRight = inputs[1][i];
			const float inputLeft  = Helpers::Mix(feedbackAmp*dryLeft,  feedbackSign*lastSample[0], feedbackScalar);
			const float inputRight = Helpers::Mix(feedbackAmp*dryRight, feedbackSign*lastSample[1], feedbackScalar);
			float stateLeft  = allpass[0][0].Next(inputLeft);
			float stateRight = allpass[1][0].Next(inputRight);

			for(int j = 1; j < filters; ++j)
			{
				allpass[0][j].CopyCoefficients(allpass[0][0]);
				allpass[1][j].CopyCoefficients(allpass[1][0]);
				stateLeft  = allpass[0][j].Next(stateLeft);
				stateRight = allpass[1][j].Next(stateRight);
			}

			lastSample[0] = stateLeft;
			lastSample[1] = stateRight;
			outputs[0][i] = Helpers::Mix(dryLeft, stateLeft, drywet);
			outputs[1][i] = Helpers::Mix(dryRight, stateRight, drywet);
		}
	}

	void Zapper::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Frequency: frequency = Helpers::ParamToFrequency(value); break;
		case ParamIndices::Q: q = Helpers::ParamToAllpassQ(value); break;
		case ParamIndices::Strength: strength = value; break;
		case ParamIndices::Feedback: feedback = value; break;
		case ParamIndices::Drywet: drywet = value; break;
		}
	}

	float Zapper::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Frequency:
		default:
			return Helpers::FrequencyToParam(frequency);

		case ParamIndices::Q: return Helpers::AllpassQToParam(q);
		case ParamIndices::Strength: return strength;
		case ParamIndices::Feedback: return feedback;
		case ParamIndices::Drywet: return drywet;
		}
	}
}
