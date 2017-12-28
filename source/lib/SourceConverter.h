#pragma once
#include "AudioVisualizer.abi.h"
#include <limits>
#include <vector>
#include <windows.ui.xaml.data.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::AudioVisualizer;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Xaml::Data;

namespace AudioVisualizer
{
	class SourceConverter : public RuntimeClass<
		IVisualizationSource,
		ISourceConverter>
	{
		ComPtr<IVisualizationSource> _source;
		AnalyzerType _analyzerTypes;
		ComPtr<IReference<UINT32>> _elementCount;
		ComPtr<IReference<UINT32>> _channelCount;
		ComPtr<IReference<TimeSpan>> _rmsRiseTime;
		ComPtr<IReference<TimeSpan>> _rmsFallTime;
		ComPtr<IReference<TimeSpan>> _peakRiseTime;
		ComPtr<IReference<TimeSpan>> _peakFallTime;
		ComPtr<IReference<TimeSpan>> _spectrumRiseTime;
		ComPtr<IReference<TimeSpan>> _spectrumFallTime;
		ComPtr<IReference<float>> _minFrequency;
		ComPtr<IReference<float>> _maxFrequency;
		ComPtr<IReference<ScaleType>> _frequencyScale;
		ComPtr<IVisualizationDataFrame> _cachedSourceFrame;
		ComPtr<IVisualizationDataFrame> _cachedOutputFrame;
		bool _bCacheData;

		ComPtr<ISpectrumData> _previousSpectrum;
		ComPtr<IScalarData> _previousRMS;
		ComPtr<IScalarData> _previousPeak;

		std::vector<float> _channelMap;	// Copy of the channel map property
		std::vector<float> _channelMappingMatrix;
		bool _bMapChannels;

		CriticalSection _csLock;
		TimeSpan _timeFromPrevious;

		EventRegistrationToken _sourceChanged;

		EventSource<ITypedEventHandler<IVisualizationSource*, HSTRING>> _configurationChangedList;
		HRESULT RaiseConfigurationChanged(wchar_t *wszPropertyName)
		{
			return _configurationChangedList.InvokeAll(this, HStringReference(wszPropertyName).Get());
		}

		InspectableClass(RuntimeClass_AudioVisualizer_SourceConverter, BaseTrust);

		HRESULT ProcessFrame(IVisualizationDataFrame *pSource, IVisualizationDataFrame **ppResult);
		HRESULT CloneScalarWithChannelCount(IScalarData *pSource, UINT32 channelCount, IScalarData **ppResult);
		HRESULT CloneSpectrumWithChannelCount(ISpectrumData *pSource, UINT32 channelCount,ISpectrumData **ppResult);
		HRESULT TryConstructingEmptySourceFrame(IVisualizationDataFrame **ppResult);
		HRESULT CloneSpectrum(ISpectrumData *pSource, ISpectrumData **ppResult);
		HRESULT CloneScalarData(IScalarData *pSource, IScalarData **ppResult);
		HRESULT CloneFromFrame(IVisualizationDataFrame *pSource, IVisualizationDataFrame **ppTarget);
		HRESULT ApplyFrequencyTransforms(ISpectrumData *pSource, ISpectrumData **ppResult);
		HRESULT ApplyRiseAndFall(ISpectrumData *pSource,ISpectrumData *pPrevious,IReference<TimeSpan> *riseTime,IReference<TimeSpan> *fallTime,ISpectrumData **ppResult);
		HRESULT ApplyRiseAndFall(IScalarData *pSource,IScalarData *pPrevious, IReference<TimeSpan> *riseTime, IReference<TimeSpan> *fallTime, IScalarData **ppResult);
		void ResetCachedItems()
		{
			_cachedOutputFrame = nullptr;
			_cachedOutputFrame = nullptr;
			_previousPeak = nullptr;
			_previousRMS = nullptr;
			_previousSpectrum = nullptr;
		}
		void BuildChannelMap();

	public:
		SourceConverter();
		~SourceConverter();

		STDMETHODIMP GetData(IVisualizationDataFrame **ppResult);
		STDMETHODIMP get_IsSuspended(boolean *pbIsSuspended)
		{
			auto lock = _csLock.Lock();
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			return _source->get_IsSuspended(pbIsSuspended);
		}
		STDMETHODIMP put_IsSuspended(boolean bIsSuspended)
		{
			auto lock = _csLock.Lock();
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			return _source->put_IsSuspended(bIsSuspended);
		}
		STDMETHODIMP get_Fps(float *pFramesPerSecond)
		{
			auto lock = _csLock.Lock();
			if (pFramesPerSecond == nullptr)
				return E_POINTER;
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			return _source->get_Fps(pFramesPerSecond);

		}
		STDMETHODIMP put_Fps(float framesPerSecond)
		{
			auto lock = _csLock.Lock();
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			return _source->put_Fps(framesPerSecond);

		}
		STDMETHODIMP get_AnalyzerTypes(AnalyzerType *pResult)
		{
			auto lock = _csLock.Lock();
			if (pResult == nullptr)
				return E_POINTER;
			*pResult = _analyzerTypes;
			return S_OK;
		}
		STDMETHODIMP put_AnalyzerTypes(AnalyzerType types)
		{
			auto lock = _csLock.Lock();
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			_analyzerTypes = types;
			return S_OK;
		}
		STDMETHODIMP get_PresentationTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			return _source->get_PresentationTime(ppTime);
		}
		STDMETHODIMP get_PlaybackState(SourcePlaybackState *pState)
		{
			auto lock = _csLock.Lock();
			if (pState == nullptr)
				return E_POINTER;
			if (_source == nullptr)
				return E_NOT_VALID_STATE;
			return _source->get_PlaybackState(pState);
		}

		STDMETHODIMP get_Source(IVisualizationSource **ppSource)
		{
			auto lock = _csLock.Lock();
			if (ppSource == nullptr)
				return E_POINTER;
			return _source.CopyTo(ppSource);
			return S_OK;
		}

		STDMETHODIMP put_Source(IVisualizationSource *pSource)
		{
			auto lock = _csLock.Lock();
			if (_source != nullptr)
			{
				_source->remove_ConfigurationChanged(_sourceChanged);
			}
			_source = pSource;
			
			if (_source != nullptr)
			{
				_source->add_ConfigurationChanged(
					Callback<ITypedEventHandler<IVisualizationSource *,HSTRING>>(
						[=](IVisualizationSource *pSender, HSTRING propertyName)->HRESULT
				{
					return _configurationChangedList.InvokeAll(pSender, propertyName);
				}
						).Get(),
					&_sourceChanged
				);
			}
			ResetCachedItems();
			RaiseConfigurationChanged(L"Source");
			return S_OK;
		}
		STDMETHODIMP get_FrequencyCount(IReference<UINT32> **ppcElements)
		{
			auto lock = _csLock.Lock();
			if (ppcElements == nullptr)
				return E_POINTER;
			return _elementCount.CopyTo(ppcElements);
		}
		STDMETHODIMP put_FrequencyCount(IReference<UINT32> *pcElements)
		{
			auto lock = _csLock.Lock();
			if (pcElements == nullptr)
				return E_POINTER;
			if (pcElements != nullptr)
			{
				UINT32 value = 0;
				pcElements->get_Value(&value);
				if (value == 0)
					return E_INVALIDARG;
			}
			_elementCount = pcElements;
			ResetCachedItems();
			RaiseConfigurationChanged(L"FrequencyCount");
			return S_OK;
		}
		STDMETHODIMP get_ChannelCount(IReference<UINT32> **ppcElements)
		{
			auto lock = _csLock.Lock();
			if (ppcElements == nullptr)
				return E_POINTER;
			return _channelCount.CopyTo(ppcElements);
		}
		STDMETHODIMP put_ChannelCount(IReference<UINT32> *pcElements)
		{
			auto lock = _csLock.Lock();
			if (pcElements == nullptr)
				return E_POINTER;
			bool bValueChanged = true;

			if (pcElements != nullptr)
			{
				UINT32 value = 0;
				pcElements->get_Value(&value);
				if (value == 0)
					return E_INVALIDARG;
				if (_channelCount != nullptr)
				{
					UINT32 oldValue = 0;
					_channelCount->get_Value(&oldValue);
					if (oldValue == value)
						bValueChanged = false;
				}
			}
			else
			{
				if (_channelCount == nullptr)
					bValueChanged = false;
			}

			if (bValueChanged)
			{
				_channelCount = pcElements;
				_channelMap.resize(0);
				ResetCachedItems();
				BuildChannelMap();
				RaiseConfigurationChanged(L"ChannelCount");
			}
			return S_OK;
		}

		STDMETHODIMP get_ChannelMapping(UINT32 *pCount, float **ppValues)
		{
			if (ppValues == nullptr || pCount == nullptr)
				return E_POINTER;

			if (_channelMap.size() != 0)
				*ppValues = _channelMap.data();
			else
				*ppValues = nullptr;
			*pCount = _channelMap.size();

			return S_OK;
		}

		/* This property is used to map input to output channels
			and will depend on ChannelCount. Will have effect if ChannelCount is set only

			for channelCount = 2

			values[0] - coefficient of first input channel into first output channel
			values[1] - coefficient of first input channel into second output channel
			values[2] - coefficient of second input channel into first output channel
			values[3] - coefficient of second input channel into second output channel
			*/
		STDMETHODIMP put_ChannelMapping(UINT32 cCount, float *pValues)
		{
			auto lock = _csLock.Lock();
			if ( pValues == nullptr)
			{
				_channelMap.resize(0);
			}
			else
			{
				if (_channelCount == nullptr)	// Cannot set without channelcount set
					return E_INVALIDARG;
				UINT32 outChannelCount;
				_channelCount->get_Value(&outChannelCount);
				if (outChannelCount > cCount)
					return E_INVALIDARG;

				_channelMap.resize(cCount);
				for (size_t index = 0; index < cCount; index++)
				{
					_channelMap[index] = pValues[index];
				}
			}
			BuildChannelMap();
			return S_OK;
		}

		STDMETHODIMP get_MinFrequency(IReference<float> **ppValue)
		{
			auto lock = _csLock.Lock();
			if (ppValue == nullptr)
				return E_POINTER;
			return _minFrequency.CopyTo(ppValue);
		}
		STDMETHODIMP put_MinFrequency(IReference<float> *pValue)
		{
			auto lock = _csLock.Lock();
			if (pValue == nullptr)
				return E_POINTER;
			if (pValue != nullptr)
			{
				float value = 0;
				pValue->get_Value(&value);
				ScaleType scale = ScaleType::Linear;
				if (_frequencyScale != nullptr)
					_frequencyScale->get_Value(&scale);
				float maxFreq = std::numeric_limits<float>::max();
				if (_maxFrequency != nullptr)
					_maxFrequency->get_Value(&maxFreq);

				if ((scale == ScaleType::Logarithmic && value == 0) || value < 0 || value >= maxFreq)
					return E_INVALIDARG;
			}
			_minFrequency = pValue;
			ResetCachedItems();
			RaiseConfigurationChanged(L"MinFrequency");
			return S_OK;
		}
		STDMETHODIMP get_MaxFrequency(IReference<float> **ppValue)
		{
			auto lock = _csLock.Lock();
			if (ppValue == nullptr)
				return E_POINTER;
			return _maxFrequency.CopyTo(ppValue);
		}
		STDMETHODIMP put_MaxFrequency(IReference<float> *pValue)
		{
			auto lock = _csLock.Lock();
			if (pValue == nullptr)
				return E_POINTER;
			if (pValue != nullptr)
			{
				float value = 0;
				pValue->get_Value(&value);
				if (value < 0)
					return E_INVALIDARG;
				if (_minFrequency != nullptr)
				{
					float minFrequency = 0.0f;
					_minFrequency->get_Value(&minFrequency);
					if (minFrequency >= value)
						return E_INVALIDARG;
				}
			}
			_maxFrequency = pValue;
			ResetCachedItems();
			RaiseConfigurationChanged(L"MaxFrequency");
			return S_OK;
		}

		STDMETHODIMP get_FrequencyScale(IReference<ScaleType> **ppValue)
		{
			auto lock = _csLock.Lock();
			if (ppValue == nullptr)
				return E_POINTER;
			return _frequencyScale.CopyTo(ppValue);
		}
		STDMETHODIMP put_FrequencyScale(IReference<ScaleType> *pValue)
		{
			auto lock = _csLock.Lock();
			if (pValue == nullptr)
				return E_POINTER;
			if (pValue != nullptr)
			{
				ScaleType value = ScaleType::Linear;
				pValue->get_Value(&value);
				if (value == ScaleType::Logarithmic && _minFrequency != nullptr)
				{
					float minFrequency = 0;
					_minFrequency->get_Value(&minFrequency);
					if (_minFrequency == 0)
						return E_INVALIDARG;
				}
			}
			_frequencyScale = pValue;
			ResetCachedItems();
			RaiseConfigurationChanged(L"FrequencyScale");
			return S_OK;
		}

		STDMETHODIMP get_RmsRiseTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			return _rmsRiseTime.CopyTo(ppTime);
		}
		STDMETHODIMP put_RmsRiseTime(IReference<TimeSpan> *pTime)
		{
			auto lock = _csLock.Lock();
			if (pTime == nullptr)
				return E_POINTER;
			if (pTime != nullptr)
			{
				TimeSpan value = { 0 };
				pTime->get_Value(&value);
				if (value.Duration == 0)
					return E_INVALIDARG;
			}
			_rmsRiseTime = pTime;
			RaiseConfigurationChanged(L"RmsRiseTime");
			return S_OK;
		}

		STDMETHODIMP get_RmsFallTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			return _rmsFallTime.CopyTo(ppTime);
		}
		STDMETHODIMP put_RmsFallTime(IReference<TimeSpan> *pTime)
		{
			auto lock = _csLock.Lock();
			if (pTime == nullptr)
				return E_POINTER;
			if (pTime != nullptr)
			{
				TimeSpan value = { 0 };
				pTime->get_Value(&value);
				if (value.Duration == 0)
					return E_INVALIDARG;
			}
			_rmsFallTime = pTime;
			RaiseConfigurationChanged(L"RmsFallTime");
			return S_OK;
		}

		STDMETHODIMP get_PeakRiseTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			return _peakRiseTime.CopyTo(ppTime);
		}
		STDMETHODIMP put_PeakRiseTime(IReference<TimeSpan> *pTime)
		{
			auto lock = _csLock.Lock();
			if (pTime == nullptr)
				return E_POINTER;
			if (pTime != nullptr)
			{
				TimeSpan value = { 0 };
				pTime->get_Value(&value);
				if (value.Duration == 0)
					return E_INVALIDARG;
			}
			_peakRiseTime = pTime;
			RaiseConfigurationChanged(L"PeakRiseTime");
			return S_OK;
		}

		STDMETHODIMP get_PeakFallTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			return _peakFallTime.CopyTo(ppTime);
		}
		STDMETHODIMP put_PeakFallTime(IReference<TimeSpan> *pTime)
		{
			auto lock = _csLock.Lock();
			if (pTime == nullptr)
				return E_POINTER;
			if (pTime != nullptr)
			{
				TimeSpan value = { 0 };
				pTime->get_Value(&value);
				if (value.Duration == 0)
					return E_INVALIDARG;
			}
			_peakFallTime = pTime;
			RaiseConfigurationChanged(L"PeakFallTime");
			return S_OK;
		}

		STDMETHODIMP get_SpectrumRiseTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			return _spectrumRiseTime.CopyTo(ppTime);
		}
		STDMETHODIMP put_SpectrumRiseTime(IReference<TimeSpan> *pTime)
		{
			auto lock = _csLock.Lock();
			if (pTime == nullptr)
				return E_POINTER;
			if (pTime != nullptr)
			{
				TimeSpan value = { 0 };
				pTime->get_Value(&value);
				if (value.Duration == 0)
					return E_INVALIDARG;
			}
			_spectrumRiseTime = pTime;
			RaiseConfigurationChanged(L"SpectrumRiseTime");
			return S_OK;
		}

		STDMETHODIMP get_SpectrumFallTime(IReference<TimeSpan> **ppTime)
		{
			auto lock = _csLock.Lock();
			if (ppTime == nullptr)
				return E_POINTER;
			return _spectrumFallTime.CopyTo(ppTime);
		}
		STDMETHODIMP put_SpectrumFallTime(IReference<TimeSpan> *pTime)
		{
			auto lock = _csLock.Lock();
			if (pTime == nullptr)
				return E_POINTER;
			if (pTime != nullptr)
			{
				TimeSpan value = { 0 };
				pTime->get_Value(&value);
				if (value.Duration == 0)
					return E_INVALIDARG;
			}
			_spectrumFallTime = pTime;
			RaiseConfigurationChanged(L"SpectrumFallTime");
			return S_OK;
		}



		STDMETHODIMP get_ActualFrequencyCount(IReference<UINT32> **ppcElements);
		STDMETHODIMP get_ActualChannelCount(IReference<UINT32> **ppcElements);
		STDMETHODIMP get_ActualMinFrequency(IReference<float> **ppValue);
		STDMETHODIMP get_ActualMaxFrequency(IReference<float> **ppValue);
		STDMETHODIMP get_ActualFrequencyScale(IReference<ScaleType> **ppValue);

		STDMETHODIMP add_ConfigurationChanged(
			ITypedEventHandler<IVisualizationSource *,HSTRING> *value,
			EventRegistrationToken *token)
		{
			auto lock = _csLock.Lock();
			return _configurationChangedList.Add(value, token);
		}
		STDMETHODIMP remove_ConfigurationChanged(
			EventRegistrationToken token)
		{
			auto lock = _csLock.Lock();
			return _configurationChangedList.Remove(token);
		}
	};
}
