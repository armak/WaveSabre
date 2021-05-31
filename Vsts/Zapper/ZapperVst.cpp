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
	: VstPlug(audioMaster, (int)Zapper::ParamIndices::NumParams, 2, 2, 'Zppr', new Zapper())
{
	setEditor(new ZapperEditor(this));
}

void ZapperVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Zapper::ParamIndices)index)
	{
	case Zapper::ParamIndices::Frequency: vst_strncpy(text, "Freq", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Q: vst_strncpy(text, "Q", kVstMaxParamStrLen); break;
	case Zapper::ParamIndices::Strength: vst_strncpy(text, "Strength", kVstMaxParamStrLen); break;
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
