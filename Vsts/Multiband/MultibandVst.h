#ifndef __MULTIBANDVST_H__
#define __MULTIBANDVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class MultibandVst : public VstPlug
{
public:
	MultibandVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif