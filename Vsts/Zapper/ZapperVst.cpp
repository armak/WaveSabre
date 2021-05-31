#include "ZapperVst.h"
#include "ZapperEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new ZapperVst(audioMaster);
}

ZapperVst::ZapperVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Zapper::ParamIndices::NumParams, 4, 2, 'Obtr', new Zapper())
{
	setEditor(new ZapperEditor(this));
}

void ZapperVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Zapper::ParamIndices)index)
	{
	case Zapper::ParamIndices::Source: vst_strncpy(text, "Sidchain", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Frequency: vst_strncpy(text, "Freq", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Shape: vst_strncpy(text, "Shape", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Bias: vst_strncpy(text, "Bias", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Rectify: vst_strncpy(text, "Rectify", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Spread: vst_strncpy(text, "Spread", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Amount: vst_strncpy(text, "Amount", kVstMaxParamStrLen); break;
	}
}

bool ZapperVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Zapper", kVstMaxEffectNameLen);
	return true;
}

bool ZapperVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Zapper", kVstMaxProductStrLen);
	return true;
}
