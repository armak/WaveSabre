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
		case Multiband::ParamIndices::Sidechain: vst_strncpy(text, "Sidchain", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::InputGain: vst_strncpy(text, "In Gain", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::LowBandCutoff: vst_strncpy(text, "Low Freq", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::MidBandCutoff: vst_strncpy(text, "Mid Freq", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::HighBandCutoff: vst_strncpy(text, "Hi Freq", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band1Threshold: vst_strncpy(text, "B1 Tresh", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band1Ratio: vst_strncpy(text, "B1 Ratio", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band1Attack: vst_strncpy(text, "B1 Atk", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band1Release: vst_strncpy(text, "B1 Rel", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band1Gain: vst_strncpy(text, "B1 Gain", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band2Threshold: vst_strncpy(text, "B2 Tresh", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band2Ratio: vst_strncpy(text, "B2 Ratio", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band2Attack: vst_strncpy(text, "B2 Atk", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band2Release: vst_strncpy(text, "B2 Rel", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band2Gain: vst_strncpy(text, "B2 Gain", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band3Threshold: vst_strncpy(text, "B3 Tresh", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band3Ratio: vst_strncpy(text, "B3 Ratio", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band3Attack: vst_strncpy(text, "B3 Atk", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band3Release: vst_strncpy(text, "B3 Rel", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band3Gain: vst_strncpy(text, "B3 Gain", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band4Threshold: vst_strncpy(text, "B4 Tresh", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band4Ratio: vst_strncpy(text, "B4 Ratio", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band4Attack: vst_strncpy(text, "B4 Atk", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band4Release: vst_strncpy(text, "B4 Rel", kVstMaxParamStrLen); break;
		case Multiband::ParamIndices::Band4Gain: vst_strncpy(text, "B4 Gain", kVstMaxParamStrLen); break;
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
