#include "OrbiterVst.h"
#include "OrbiterEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new OrbiterVst(audioMaster);
}

OrbiterVst::OrbiterVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Orbiter::ParamIndices::NumParams, 4, 2, 'Obtr', new Orbiter())
{
	setEditor(new OrbiterEditor(this));
}

void OrbiterVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Orbiter::ParamIndices)index)
	{
	case Orbiter::ParamIndices::Source: vst_strncpy(text, "Sidchain", kVstMaxParamStrLen); break;
	case Orbiter::ParamIndices::Frequency: vst_strncpy(text, "Freq", kVstMaxParamStrLen); break;
	case Orbiter::ParamIndices::Shape: vst_strncpy(text, "Shape", kVstMaxParamStrLen); break;
	case Orbiter::ParamIndices::Bias: vst_strncpy(text, "Bias", kVstMaxParamStrLen); break;
	case Orbiter::ParamIndices::Rectify: vst_strncpy(text, "Rectify", kVstMaxParamStrLen); break;
	case Orbiter::ParamIndices::Spread: vst_strncpy(text, "Spread", kVstMaxParamStrLen); break;
	case Orbiter::ParamIndices::Amount: vst_strncpy(text, "Amount", kVstMaxParamStrLen); break;
	}
}

bool OrbiterVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Orbiter", kVstMaxEffectNameLen);
	return true;
}

bool OrbiterVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Orbiter", kVstMaxProductStrLen);
	return true;
}
