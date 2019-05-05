#include "ShimmerEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

ShimmerEditor::ShimmerEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 760, 220, "SHIMMER")
{
}

ShimmerEditor::~ShimmerEditor()
{
}

void ShimmerEditor::Open()
{
	addKnob((VstInt32)Shimmer::ParamIndices::Osc1Pitch, "OCTAVE");
	addKnob((VstInt32)Shimmer::ParamIndices::Osc1Finetune, "PITCH");
	//addKnob((VstInt32)Shimmer::ParamIndices::Osc1Volume, "VOLUME");
	
	addSpacer();
	addSpacer();

	addKnob((VstInt32)Shimmer::ParamIndices::Attack, "ATTACK");
	addKnob((VstInt32)Shimmer::ParamIndices::Decay, "DECAY");
	addKnob((VstInt32)Shimmer::ParamIndices::Sustain, "SUSTAIN");
	addKnob((VstInt32)Shimmer::ParamIndices::Release, "RELEASE");

	addSpacer();
	addSpacer();

	addKnob((VstInt32)Shimmer::ParamIndices::MasterLevel, "MASTER");

	startNextRow();


	addKnob((VstInt32)Shimmer::ParamIndices::Osc1Partials, "PARTIALS");
	addKnob((VstInt32)Shimmer::ParamIndices::PartialSpacing, "SPACING");
	addKnob((VstInt32)Shimmer::ParamIndices::Osc1Pattern, "PATTERN");
	addKnob((VstInt32)Shimmer::ParamIndices::PartialCoarse, "SHUFFLE");
	addKnob((VstInt32)Shimmer::ParamIndices::PartialFine, "DRIFT");
	addKnob((VstInt32)Shimmer::ParamIndices::PartialSpread, "WIDTH");
	addSpacer();
	addKnob((VstInt32)Shimmer::ParamIndices::PartialModFreq, "MOD FREQ");
	addKnob((VstInt32)Shimmer::ParamIndices::PartialModDepth, "MOD DEPTH");

	addSpacer();
	addSpacer();

	startNextRow();

	addKnob((VstInt32)Shimmer::ParamIndices::VoicesUnisono, "UNISONO");
	addKnob((VstInt32)Shimmer::ParamIndices::VoicesDetune, "DETUNE");
	addKnob((VstInt32)Shimmer::ParamIndices::VoicesPan, "PAN");

	addSpacer();

	addKnob((VstInt32)Shimmer::ParamIndices::VibratoFreq, "VIB FREQ");
	addKnob((VstInt32)Shimmer::ParamIndices::VibratoAmount, "VIB AMT");

	addSpacer();

	addKnob((VstInt32)Shimmer::ParamIndices::Rise, "RISE");

	addSpacer();
	addKnob((VstInt32)Shimmer::ParamIndices::PitchAttack, "P ATTACK");
	addKnob((VstInt32)Shimmer::ParamIndices::PitchDecay, "P DECAY");
	addKnob((VstInt32)Shimmer::ParamIndices::PitchSustain, "P SUSTAIN");
	addKnob((VstInt32)Shimmer::ParamIndices::PitchRelease, "P RELEASE");
	addKnob((VstInt32)Shimmer::ParamIndices::PitchEnvAmt, "P AMT 1");

	VstEditor::Open();
}
