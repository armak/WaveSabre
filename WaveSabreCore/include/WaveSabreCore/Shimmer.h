#ifndef __WAVESABRECORE_SHIMMER_H__
#define __WAVESABRECORE_SHIMMER_H__

#include "SynthDevice.h"
#include "Envelope.h"

namespace WaveSabreCore
{
	class Shimmer : public SynthDevice
	{
	public:
		enum class ParamIndices
		{
			Osc1Pitch,
			Osc1Finetune,
			Osc1Volume,
			Osc1Partials,
			Osc1Pattern,

			Attack,
			Decay,
			Sustain,
			Release,

			PartialSpacing,
			PartialCoarse,
			PartialFine,
			PartialSpread,
			PartialModFreq,
			PartialModDepth,

			MasterLevel,

			VoicesUnisono,
			VoicesDetune,
			VoicesPan,

			VibratoFreq,
			VibratoAmount,

			Rise,

			PitchAttack,
			PitchDecay,
			PitchSustain,
			PitchRelease,
			PitchEnvAmt,

			NumParams,
		};

		Shimmer();

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	protected:
		class ShimmerVoice : public Voice
		{
		public:
			ShimmerVoice(Shimmer *shimmer);
			virtual WaveSabreCore::SynthDevice *SynthDevice() const;

			virtual void Run(double songPosition, float **outputs, int numSamples);

			virtual void NoteOn(int note, int velocity, float detune, float pan);
			virtual void NoteOff();

		private:
			Shimmer *shimmer;

			Envelope osc1Env, pitchEnv;

			double osc1Phase;
			double osc1Output;
		};

		static double ratioScalar(double coarse, double fine);

		float osc1Octave, osc1Pitch, osc1Volume, osc1Partials, osc1Pattern;
		//float osc1Pitch, osc1Fine, osc1Volume, osc1Partials, osc1Pattern;
		float attack, decay, sustain, release;
		//float partialAttack, partialDecay, partialSustain, partialRelease;
		float partialSpacing, partialCoarse, partialFine, partialSpread, partialModFreq, partialModDepth;
		float masterLevel;
		float pitchAttack, pitchDecay, pitchSustain, pitchRelease, pitchEnvAmt;

	private:
		static const int TableLength = 256;
		float partialVolTableA[TableLength];
		float partialVolTableB[TableLength];
		float partialShiftTableA[TableLength];
		float partialShiftTableB[TableLength];
		float partialCoarseTable[TableLength];

		double modulationPhase;
	};
}

#endif
