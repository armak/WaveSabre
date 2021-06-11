#ifndef __WAVESABRECORE_MULTIBAND_H__
#define __WAVESABRECORE_MULTIBAND_H__

#include "Device.h"
#include "DelayBuffer.h"

namespace WaveSabreCore
{
	class Multiband : public Device
	{
	public:
		enum class ParamIndices
		{
			NumParams,
		};

		Multiband();
		virtual ~Multiband();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
	};
}

#endif
