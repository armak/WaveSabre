#ifndef __SHIMMERVST_H__
#define __SHIMMERVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class ShimmerVst : public VstPlug
{
public:
	ShimmerVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif
