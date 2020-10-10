#ifndef __BUTTERVST_H__
#define __BUTTERVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class ButterVst : public VstPlug
{
public:
	ButterVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif