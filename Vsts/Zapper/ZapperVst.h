#ifndef __ZAPPERVST_H__
#define __ZAPPERVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class ZapperVst : public VstPlug
{
public:
	ZapperVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif