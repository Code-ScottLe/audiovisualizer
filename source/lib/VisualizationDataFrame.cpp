#include "pch.h"
#include "VisualizationDataFrame.h"

namespace AudioVisualizer
{
	class VisualizationDataFrameFactory : public AgileActivationFactory<IVisualizationDataFrameFactory>
	{
	public:
		STDMETHODIMP Create(TimeSpan time, TimeSpan duration, IScalarData *pRms, IScalarData *pPeak, ISpectrumData *pSpectrum,IVisualizationDataFrame **ppResult)
		{
			if (ppResult == nullptr)
				return E_POINTER;
			ComPtr<VisualizationDataFrame> frame = Make<VisualizationDataFrame>(time.Duration, duration.Duration, pRms, pPeak, pSpectrum);
			return frame.CopyTo(ppResult);
		}
	};

	VisualizationDataFrame::VisualizationDataFrame(REFERENCE_TIME time, REFERENCE_TIME duration, IScalarData *pRms, IScalarData *pPeak, ISpectrumData *pSpectrum)
	{
		_time.Duration = time;
		_duration.Duration = duration;
		_rms = pRms;
		_peak = pPeak;
		_spectrum = pSpectrum;
	}

	VisualizationDataFrame::~VisualizationDataFrame()
	{
		_rms = nullptr;
		_peak = nullptr;
		_spectrum = nullptr;
	}

	STDMETHODIMP VisualizationDataFrame::CombineChannels(UINT32 elementCount, float * pMap, UINT32 cChannels, IVisualizationDataFrame ** ppResult)
	{
		if (pMap == nullptr || ppResult == nullptr)
			return E_POINTER;
		if (cChannels == 0)
			return E_INVALIDARG;
		
		ComPtr<IScalarData> newRms;
		ComPtr<IScalarData> newPeak;
		ComPtr<ISpectrumData> newSpectrum;
		UINT32 channels = 0;
		if (_rms != nullptr || _peak != nullptr || _spectrum != nullptr)
		{
			if (_rms != nullptr)
			{
				ComPtr<IVectorView<float>> vector;
				_rms.As(&vector);
				if (vector != nullptr)
				{
					vector->get_Size(&channels);
				}
			}
			if (_peak != nullptr)
			{
				ComPtr<IVectorView<float>> vector;
				_peak.As(&vector);
				if (vector != nullptr)
				{
					UINT32 size = 0;
					vector->get_Size(&size);
					if (channels != 0 && size != channels)
						return E_NOT_VALID_STATE;
					channels = size;
				}
			}
			if (_spectrum != nullptr)
			{
				ComPtr<IVectorView<IVectorView<float>*>> vector;
				_spectrum.As(&vector);
				if (vector != nullptr)
				{
					UINT32 size = 0;
					vector->get_Size(&size);
					if (channels != 0 && size != channels)
						return E_NOT_VALID_STATE;
					channels = size;
				}
			}
		}

		if (channels != 0)
		{
			if (channels * cChannels != elementCount)
				return E_INVALIDARG;

			if (_rms != nullptr)
				_rms->CombineChannels(elementCount, pMap, cChannels,&newRms);
			if (_peak != nullptr)
				_peak->CombineChannels(elementCount, pMap, cChannels, &newPeak);
			if (_spectrum != nullptr)
				_spectrum->CombineChannels(elementCount, pMap, cChannels, &newSpectrum);
		}

		ComPtr<VisualizationDataFrame> resultFrame = Make<VisualizationDataFrame>(
			_time.Duration,
			_duration.Duration,
			newRms.Get(),
			newPeak.Get(),
			newSpectrum.Get()
			);

		return resultFrame.CopyTo(ppResult);

	}

	ActivatableClassWithFactory(VisualizationDataFrame, VisualizationDataFrameFactory);
}
