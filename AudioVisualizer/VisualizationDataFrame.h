#pragma once

#define _CRTDBG_MAP_ALLOC 

#include "AudioVisualizer_h.h"
#include "VisualizationData.h"
#include "Nullable.h"
#include <DirectXMath.h>

using namespace ABI::AudioVisualizer;
using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;

namespace AudioVisualizer
{
	class VisualizationDataFrame : public RuntimeClass<IVisualizationDataFrame>
	{
		InspectableClass(RuntimeClass_AudioVisualizer_VisualizationDataFrame, BaseTrust)
		ComPtr<IReference<TimeSpan>> _time;
		ComPtr<IReference<TimeSpan>> _duration;
		ComPtr<ScalarData> _rms;
		ComPtr<ScalarData> _peak;
		ComPtr<VectorData> _spectrum;

	public:
		VisualizationDataFrame(IReference<TimeSpan> *pTime, IReference<TimeSpan> *pDuration, ScalarData *pRms, ScalarData *pPeak, VectorData *pSpectrum)
		{
			_time = pTime;
			_duration = pDuration;
			_rms = pRms;
			_peak = pPeak;
			_spectrum = pSpectrum;
		}

		STDMETHODIMP get_Time(ABI::Windows::Foundation::IReference<ABI::Windows::Foundation::TimeSpan> **ppTimeStamp)
		{
			if (ppTimeStamp == nullptr)
				return E_INVALIDARG;
			_time.CopyTo(ppTimeStamp);
			return S_OK;
		}
		STDMETHODIMP get_Duration(ABI::Windows::Foundation::IReference<ABI::Windows::Foundation::TimeSpan> **ppTimeStamp)
		{
			if (ppTimeStamp == nullptr)
				return E_INVALIDARG;
			_duration.CopyTo(ppTimeStamp);
			return S_OK;
		}
		STDMETHODIMP get_RMS(IVisualizationData **ppData)
		{
			if (ppData == nullptr)
				return E_INVALIDARG;
			_rms.CopyTo(ppData);
			return S_OK;
		}
		STDMETHODIMP get_Peak(IVisualizationData **ppData)
		{
			if (ppData == nullptr)
				return E_INVALIDARG;
			_peak.CopyTo(ppData);
			return S_OK;
		}
		STDMETHODIMP get_Spectrum(IVisualizationData **ppData)
		{
			if (ppData == nullptr)
				return E_INVALIDARG;
			_spectrum.CopyTo(ppData);
			return S_OK;
		}
	};
}

