#include "MultibandVst.h"
#include "MultibandEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new MultibandVst(audioMaster);
}

MultibandVst::MultibandVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Multiband::ParamIndices::NumParams, 4, 2, 'Mltb', new Multiband())
{
	setEditor(new MultibandEditor(this));
}

void MultibandVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Multiband::ParamIndices)index)
	{
	}
}

bool MultibandVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Multiband", kVstMaxEffectNameLen);
	return true;
}

bool MultibandVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Multiband", kVstMaxProductStrLen);
	return true;
}
