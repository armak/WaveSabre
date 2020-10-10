#include "ButterVst.h"
#include "ButterEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new ButterVst(audioMaster);
}

ButterVst::ButterVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Butter::ParamIndices::NumParams, 2, 2, 'Bttr', new Butter())
{
	setEditor(new ButterEditor(this));
}

void ButterVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Butter::ParamIndices)index)
	{
	case Butter::ParamIndices::Cutoff: vst_strncpy(text, "Cutoff", kVstMaxParamStrLen); break;
	case Butter::ParamIndices::Q: vst_strncpy(text, "Q", kVstMaxParamStrLen); break;
	//case Butter::ParamIndices::DryWet: vst_strncpy(text, "Dry/Wet", kVstMaxParamStrLen); break;
	}
}

bool ButterVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Butter", kVstMaxEffectNameLen);
	return true;
}

bool ButterVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Butter", kVstMaxProductStrLen);
	return true;
}
