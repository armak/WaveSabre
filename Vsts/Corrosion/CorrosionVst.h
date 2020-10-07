#ifndef __CORROSIONVST_H__
#define __CORROSIONVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class CorrosionVst : public VstPlug
{
public:
	CorrosionVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif