#include "CorrosionVst.h"
#include "CorrosionEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new CorrosionVst(audioMaster);
}

CorrosionVst::CorrosionVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Corrosion::ParamIndices::NumParams, 2, 2, 'Crsn', new Corrosion())
{
	setEditor(new CorrosionEditor(this));
}

void CorrosionVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Corrosion::ParamIndices)index)
	{
	case Corrosion::ParamIndices::InputGain: vst_strncpy(text, "Gain", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Even: vst_strncpy(text, "Even", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Twist: vst_strncpy(text, "Twist", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Fold: vst_strncpy(text, "Fold", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Saturation: vst_strncpy(text, "Saturation", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Oversampling: vst_strncpy(text, "Oversampling", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::DryWet: vst_strncpy(text, "Dry/Wet", kVstMaxParamStrLen); break;
	}
}

bool CorrosionVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Corrosion", kVstMaxEffectNameLen);
	return true;
}

bool CorrosionVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Corrosion", kVstMaxProductStrLen);
	return true;
}
