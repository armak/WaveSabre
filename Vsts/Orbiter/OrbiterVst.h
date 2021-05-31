#ifndef __ORBITERVST_H__
#define __ORBITERVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class OrbiterVst : public VstPlug
{
public:
	OrbiterVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif