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
	case Corrosion::ParamIndices::InputGain: vst_strncpy(text, "Input", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Rectify: vst_strncpy(text, "Rectify", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Twist: vst_strncpy(text, "Twist", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Fold: vst_strncpy(text, "Fold", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Saturation: vst_strncpy(text, "Saturate", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::ClipDrive: vst_strncpy(text, "Drive", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::ClipThreshold: vst_strncpy(text, "Thres", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::ClipShape: vst_strncpy(text, "Shape", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::OutputGain: vst_strncpy(text, "Output", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::DryWet: vst_strncpy(text, "Dry/Wet", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::Oversampling: vst_strncpy(text, "Quality", kVstMaxParamStrLen); break;
	case Corrosion::ParamIndices::DCBlocking: vst_strncpy(text, "DC Block", kVstMaxParamStrLen); break;
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
