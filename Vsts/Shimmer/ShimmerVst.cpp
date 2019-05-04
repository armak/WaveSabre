#include "ShimmerVst.h"
#include "ShimmerEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new ShimmerVst(audioMaster);
}

ShimmerVst::ShimmerVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Shimmer::ParamIndices::NumParams, 0, 2, 'Shmr', new Shimmer(), true)
{
	setEditor(new ShimmerEditor(this));
}

void ShimmerVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Shimmer::ParamIndices)index)
	{
	case Shimmer::ParamIndices::Osc1Pitch: vst_strncpy(text, "Osc1 Wf", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Osc1Finetune: vst_strncpy(text, "Osc1 Rc", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Osc1Volume: vst_strncpy(text, "Osc1 Rf", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Osc1Partials: vst_strncpy(text, "Osc1 Fb", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Osc1Pattern: vst_strncpy(text, "Osc1 Ff", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::Attack: vst_strncpy(text, "Osc1 Atk", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Decay: vst_strncpy(text, "Osc1 Dcy", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Sustain: vst_strncpy(text, "Osc1 Sus", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::Release: vst_strncpy(text, "Osc1 Rls", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::PartialSpacing: vst_strncpy(text, "Osc1 Atk", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PartialCoarse: vst_strncpy(text, "Osc1 Dcy", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PartialFine: vst_strncpy(text, "Osc1 Sus", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PartialSpread: vst_strncpy(text, "Osc1 Sus", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PartialModFreq: vst_strncpy(text, "Osc1 Rls", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PartialModDepth: vst_strncpy(text, "Osc1 Rls", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::MasterLevel: vst_strncpy(text, "Mstr Lvl", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::VoicesUnisono: vst_strncpy(text, "Vcs Uni", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::VoicesDetune: vst_strncpy(text, "Vcs Det", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::VoicesPan: vst_strncpy(text, "Vcs Pan", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::VibratoFreq: vst_strncpy(text, "Vib Frq", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::VibratoAmount: vst_strncpy(text, "Vib Amt", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::Rise: vst_strncpy(text, "Rise", kVstMaxParamStrLen); break;

	case Shimmer::ParamIndices::PitchAttack: vst_strncpy(text, "Ptch Atk", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PitchDecay: vst_strncpy(text, "Ptch Dcy", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PitchSustain: vst_strncpy(text, "Ptch Sus", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PitchRelease: vst_strncpy(text, "Ptch Rls", kVstMaxParamStrLen); break;
	case Shimmer::ParamIndices::PitchEnvAmt: vst_strncpy(text, "Ptc Env1", kVstMaxParamStrLen); break;

	}
}

bool ShimmerVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Shimmer", kVstMaxEffectNameLen);
	return true;
}

bool ShimmerVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Shimmer", kVstMaxProductStrLen);
	return true;
}
