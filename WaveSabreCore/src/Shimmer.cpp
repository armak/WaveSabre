#include <WaveSabreCore/Shimmer.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Shimmer::Shimmer()
		: SynthDevice((int)ParamIndices::NumParams)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new ShimmerVoice(this);

		osc1Pitch = 0.5f;
		osc1Finetune = 0.5f;
		osc1Volume = 0.5f;
		osc1Partials = 0.25f;
		osc1Pattern = 0.0f;

		partialSpacing = 0.0f;
		partialCoarse = 0.0f;
		partialFine = 0.0f;
		partialSpread = 0.5f;
		partialModFreq = 0.0f;
		partialModDepth = 0.0f;

		attack = 1.0f;
		decay = 1.0f;
		sustain = 1.0f;
		release = 1.0f;

		masterLevel = 1.0f;

		pitchAttack = 1.0f;
		pitchDecay = 5.0f;
		pitchSustain = .5f;
		pitchRelease = 1.5f;
		pitchEnvAmt = 0.0f;

		partialVolTableA[0] = 0.5f;
		partialVolTableB[0] = 0.5f;
		partialShiftTableA[0] = 0.0f;
		partialShiftTableB[0] = 0.0f;
		partialCoarseTable[0] = 0.0f;

		for (int i = 1; i < TableLength; ++i)
		{
			partialVolTableA[i] = 0.5f + 0.5f*Helpers::RandFloat();
			partialVolTableB[i] = 0.5f + 0.5f*Helpers::RandFloat();
			partialShiftTableA[i] = Helpers::RandFloat();
			partialShiftTableB[i] = Helpers::RandFloat();
			partialCoarseTable[i] = Helpers::RandFloat();
			
		}

		modulationPhase = 0.0;
	}

	void Shimmer::SetParam(int index, float value)
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Osc1Pitch: osc1Pitch = value; break;
		case ParamIndices::Osc1Finetune: osc1Finetune = value; break;
		case ParamIndices::Osc1Volume: osc1Volume = value; break;
		case ParamIndices::Osc1Partials: osc1Partials = value; break;
		case ParamIndices::Osc1Pattern: osc1Pattern = value; break;

		case ParamIndices::Attack: attack = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::Decay: decay = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::Sustain: sustain = value; break;
		case ParamIndices::Release: release = Helpers::ScalarToEnvValue(value); break;

		case ParamIndices::PartialSpacing: partialSpacing = value; break;
		case ParamIndices::PartialCoarse: partialCoarse = value; break;
		case ParamIndices::PartialFine: partialFine = value; break;
		case ParamIndices::PartialSpread: partialSpread = value; break;
		case ParamIndices::PartialModFreq: partialModFreq = value; break;
		case ParamIndices::PartialModDepth: partialModDepth = value; break;

		case ParamIndices::MasterLevel: masterLevel = value; break;

		case ParamIndices::VoicesUnisono: VoicesUnisono = Helpers::ParamToUnisono(value); break;
		case ParamIndices::VoicesDetune: VoicesDetune = value; break;
		case ParamIndices::VoicesPan: VoicesPan = value; break;

		case ParamIndices::VibratoFreq: VibratoFreq = Helpers::ParamToVibratoFreq(value); break;
		case ParamIndices::VibratoAmount: VibratoAmount = value; break;

		case ParamIndices::Rise: Rise = value; break;

		case ParamIndices::PitchAttack: pitchAttack = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::PitchDecay: pitchDecay = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::PitchSustain: pitchSustain = value; break;
		case ParamIndices::PitchRelease: pitchRelease = Helpers::ScalarToEnvValue(value); break;
		case ParamIndices::PitchEnvAmt: pitchEnvAmt = (value - .5f) * 2.0f * 36.0f; break;
		}
	}

	float Shimmer::GetParam(int index) const
	{
		switch ((ParamIndices)index)
		{
		case ParamIndices::Osc1Pitch:
		default:
			return osc1Pitch;
		case ParamIndices::Osc1Finetune: return osc1Finetune;
		case ParamIndices::Osc1Volume: return osc1Volume;
		case ParamIndices::Osc1Partials: return osc1Partials;
		case ParamIndices::Osc1Pattern: return osc1Pattern;

		case ParamIndices::Attack: return Helpers::EnvValueToScalar(attack);
		case ParamIndices::Decay: return Helpers::EnvValueToScalar(decay);
		case ParamIndices::Sustain: return sustain;
		case ParamIndices::Release: return Helpers::EnvValueToScalar(release);

		case ParamIndices::PartialSpacing: return partialSpacing;
		case ParamIndices::PartialCoarse: return partialCoarse;
		case ParamIndices::PartialFine: return partialFine;
		case ParamIndices::PartialSpread: return partialSpread;
		case ParamIndices::PartialModFreq: return partialModFreq;
		case ParamIndices::PartialModDepth: return partialModDepth;

		case ParamIndices::MasterLevel: return masterLevel;

		case ParamIndices::VoicesUnisono: return Helpers::UnisonoToParam(VoicesUnisono);
		case ParamIndices::VoicesDetune: return VoicesDetune;
		case ParamIndices::VoicesPan: return VoicesPan;

		case ParamIndices::VibratoFreq: return Helpers::VibratoFreqToParam(VibratoFreq);
		case ParamIndices::VibratoAmount: return VibratoAmount;

		case ParamIndices::Rise: return Rise;


		case ParamIndices::PitchAttack: return Helpers::EnvValueToScalar(pitchAttack);
		case ParamIndices::PitchDecay: return Helpers::EnvValueToScalar(pitchDecay);
		case ParamIndices::PitchSustain: return pitchSustain;
		case ParamIndices::PitchRelease: return Helpers::EnvValueToScalar(pitchRelease);
		case ParamIndices::PitchEnvAmt: return pitchEnvAmt / 36.0f / 2.0f + .5f;

		}
	}

	Shimmer::ShimmerVoice::ShimmerVoice(Shimmer *shimmer)
	{
		this->shimmer = shimmer;
		this->shimmer->modulationPhase = 2.0f*3.14159f*(0.5f+0.5f*Helpers::RandFloat());
	}

	SynthDevice *Shimmer::ShimmerVoice::SynthDevice() const
	{
		return shimmer;
	}

	void Shimmer::ShimmerVoice::Run(double songPosition, float **outputs, int numSamples)
	{
		const float masterLevelScalar = Helpers::VolumeToScalar(shimmer->masterLevel);
		const double vibratoFreq = shimmer->VibratoFreq / Helpers::CurrentSampleRate;

		const float leftPanScalar = Helpers::PanToScalarLeft(Pan);
		const float rightPanScalar = Helpers::PanToScalarRight(Pan);

		const double osc1RatioScalar = ratioScalar((double)shimmer->osc1Pitch, (double)shimmer->osc1Finetune);

		double partialsd = 255.0f * shimmer->osc1Partials;
		const double partialFract = modf(partialsd, &partialsd);
		const int partials = static_cast<int>(partialsd);

		double spacingd = 11.0f*shimmer->partialSpacing;
		const double spacingFract = modf(spacingd, &spacingd);
		const int spacing = 1 + static_cast<int>(0.5f + spacingd);

		const float pattern = shimmer->osc1Pattern;

		double baseNote = GetNote() + Detune + shimmer->Rise * 24.0f;

		// Center frequency for the spread parameter
		// TODO: completely wrong at the moment
		float center = fminf(Helpers::CurrentSampleRate * 0.5, Helpers::Mix(baseNote, baseNote+static_cast<double>(partials*spacing), 0.5f));

		for (int i = 0; i < numSamples; i++)
		{
			// Modulation LFO
			shimmer->modulationPhase += 2.0*3.14159*shimmer->partialModFreq / Helpers::CurrentSampleRate;
			float currentMod = Helpers::FastSin(shimmer->modulationPhase)*shimmer->partialModDepth;
			float modInterpolation = Helpers::Mix(0.0f, currentMod, shimmer->partialModDepth);

			int index = 0;
			for (int p = 0; p <= partials*spacing; p += spacing)
			{
				// Modulation stuff
				const float strength = Helpers::Mix(shimmer->partialVolTableA[index], shimmer->partialVolTableB[index], modInterpolation);
				const float shift = (Helpers::Mix(shimmer->partialShiftTableA[index], shimmer->partialShiftTableB[index], 0.0f)) * shimmer->partialFine * spacing;
				// Shift only the harmonics, not the fundamental
				if(p > 0)
					p += static_cast<int>((static_cast<double>(spacing - 1))*(shimmer->partialCoarseTable[index])*shimmer->partialCoarse);

				double overtone = static_cast<double>(p+1);
				overtone += shift;

				overtone = Helpers::Mix(overtone, center, 1.0f - 2.0f*shimmer->partialSpread);
				const double frequency = Helpers::NoteToFreq(baseNote)*overtone;
				// Band-limiting (not sure if works properly yet, too many parameters affecting the pitch)
				if (frequency > Helpers::CurrentSampleRate * 0.5)
					break;

				const double osc1Input = osc1Phase / Helpers::CurrentSampleRate * 2.0 * 3.141592 * overtone;
				float partial = Helpers::FastCos(osc1Input) * osc1Env.GetValue() * 13.25;

				// Use the fracional component of of the partial count (a floating point value)
				// to change the amplitude of the last partial so that they don't just "pop"
				if(p == partials * spacing)
					partial *= partialFract;

				osc1Output += partial * Helpers::Mix(0.5f, strength, pattern);
				index += 1;
			}

			float finalOutput = (float)osc1Output * masterLevelScalar * 0.01f;
			outputs[0][i] += finalOutput * leftPanScalar;
			outputs[1][i] += finalOutput * rightPanScalar;

			osc1Env.Next();
			if (osc1Env.State == EnvelopeState::Finished)
			{
				IsOn = false;
				break;
			}

			float pEnv = pitchEnv.GetValue();
			double freq1 = Helpers::NoteToFreq(baseNote + pEnv * shimmer->pitchEnvAmt + Helpers::FastSin(vibratoPhase) * shimmer->VibratoAmount);
			vibratoPhase += vibratoFreq;
			osc1Phase += freq1 * osc1RatioScalar;
			// Copied from Falcon, I think this is wrong here (i.e. call this just once)
			//osc1Env.Next();
			pitchEnv.Next();
		}
	}

	void Shimmer::ShimmerVoice::NoteOn(int note, int velocity, float detune, float pan)
	{
		Voice::NoteOn(note, velocity, detune, pan);
		osc1Phase = (double)Helpers::RandFloat();
		osc1Env.Attack = shimmer->attack;
		osc1Env.Decay = shimmer->decay;
		osc1Env.Sustain = shimmer->sustain;
		osc1Env.Release = shimmer->release;
		osc1Env.Trigger();

		pitchEnv.Attack = shimmer->pitchAttack;
		pitchEnv.Decay = shimmer->pitchDecay;
		pitchEnv.Sustain = shimmer->pitchSustain;
		pitchEnv.Release = shimmer->pitchRelease;
		pitchEnv.Trigger();

		osc1Output = 0.0;
	}

	void Shimmer::ShimmerVoice::NoteOff()
	{
		osc1Env.Off();
		pitchEnv.Off();
	}

	double Shimmer::ratioScalar(double coarse, double fine)
	{
		double fineBase = (fine - .5) * 2.0;
		return 1.0 + floor(coarse * 32.99) + fineBase * fineBase * fineBase;
	}
}
