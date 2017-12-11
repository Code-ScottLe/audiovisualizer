# Build status
 

## Build status

| Target | Branch | Status | Recommended Nuget packages version |
| ------ | ------ | ------ | ------ |
| Current Release | master | [![Build status](https://ci.appveyor.com/api/projects/status/p4a7jmpa9391bxu8/branch/master?svg=true)](https://ci.appveyor.com/api/projects/status/p4a7jmpa9391bxu8/branch/master?svg=true) | [![NuGet](https://img.shields.io/nuget/v/UWPAudioVisualizer.svg)](https://www.nuget.org/packages/UWPAudioVisualizer/) |
| Pre-release | dev | [![Build status](https://ci.appveyor.com/api/projects/status/p4a7jmpa9391bxu8?svg=true)](https://ci.appveyor.com/project/clarkezone/audiovisualizer) | |

# Introduction

The AudioAnalyzer UWP extension DLL contains a component that can provide realtime audio analysis information for visualization and other purposes. Library does contain also prebuilt controls implementing VU meters and spectrum analyzer and a control that has a custom draw capability

# Getting Started
TODO: Guide users through getting your code up and running on their own system. In this section you can talk about:
1.	Installation process
2.	Software dependencies
3.	Latest releases
4.	API references


#Getting started
## Installing the library
Download and install the AudioAnalyzer nuget package, for convenient use add namespace statement to your C# code as:

`using AudioVisualizer;

add namespace statement to your XAML code as:
```
<Page ...
    xmlns:a="using:AudioVisualizer" ...>
	...
	<a:CustomVisualizer/>
	...
</Page>
```
##Initializing the source
First you need to create the analyzer object, that implements IVisualizationSource interface which is basis to retrieve audio data.

To use visualizations with MediaPlayer or MediaElement you need to use PlaybackSource class. Both MediaPlayer and MediaElement use Media Foundation pipeline to render media. Pipeline will re-create the Media Foundation Tranform object that does the analysis every time you open a new source. That means that the instance of IVisualizationSource will change for every new media that you open in MediaPlayer.

PlaybackSource monitors media pipeline and get's the new visualization when it is created and raises an event. You should use this event to get the new visualization source and set the source for all the visualization controls you have active. This is also a good place to configure the analyzer if you wish - for more on this please see Configuring the Analyzer.
If the player is a global object it is also good idea to set the current visualization source in OnLoad or OnNavigatedTo type of an event/overload.

    class VisualizationPage
    {
        MediaPlayer _player;
        AudioVisualizer.PlaybackSource _source;

        public VisualizationPage()
        {
            _player = new MediaPlayer();
            _source = new PlaybackSource(_player);
            _source.SourceChanged += Playback_SourceChanged;
		}

		private void Playback_SourceChanged(object sender, IVisualizationSource source)
        {
            ((ISpectralAnalyzerSettings)source).ConfigureSpectrum(4096, 0.5f);
			vuMeter.Source = source;
			spectrumDisplay.Source = source;
			playPositionDisplay.Source = source;
        }

		private void Page_Loaded(object sender, RoutedEventArgs e)
        {
			vuMeter.Source = _source.VisualizationSource;
			spectrumDisplay.Source = _source.VisualizationSource;
			playPositionDisplay.Source = _source.VisualizationSource;
        }
   }

## CustomVisualizer
CustomVisualizer is a control which gets draw event callback every frame and it is up for the code in the event handler to present the visual look of the control. You can use then the [VisualizerDrawEventArgs]

#API Reference
## ArrayData class
 [ArrayData](wiki/ArrayData.md)
ArrayData class is used to process spectrum data. The instance of this class is created by the library and you rarely want to create your own.

###Interfaces
- IReadOnlyList<IReadOnlyList<float>>
- IEnumerable<IReadOnlyList<float>>

### Properties
#### AmplitudeScale (ScaleType)
- Linear: Frequency amplitudes are represented by linear scale 0.0f - 1.0f
- Logarithmic: Frequency amplitudes are represented in db, where value 1.0f = 0dB

#### FrequencyCount (uint)
The count of frequency bins in spectrum.

#### FrequencyScale (ScaleType)
- Linear: Frequency scale is linear
- Logarithmic: Frequency scale is logarithmic

#### FrequencyStep (float)
Frequency value difference (linear) or ratio (logarithmic) between spectrum elements

#### MinFrequency (float)
Spectrum element 0 starting frequency

#### MaxFrequency (float)
Spectrum element FrequencyCount starting frequency

### Methods
#### ApplyRiseAndFall
This method allows to apply rise and fall times to the spectrum data over time and thus smooth the spectrum movements over time. For calculation you need to keep the value of last displayed spectrum values and pass them as the first parameter of the method. 
The time value of rise (and fall) time indicate the time by what the difference in values of spectrum data has decreased by 67%. So if the input for spectrum bin 1kHz changes from 0 to 1.0 at 1sec and the riseTime = 300ms then by 1.3sec the output value of this method at 1khZ will be 0.67
This method will fail if the AmplitudeScale is Logarithmic or the Frequency Counts and other attributes do not match

##### Parameters
- previousData (ArrayData) Spectrum values of the previous instance. If null calculations run as if all values of previous data were 0
- riseTime (TimeSpan) Time constant for rising values
- fallTime (TimeSpan) Time constant for falling values
- timeFromPrevious (TimeSpan) Time passed from previousData
##### Returns (ArrayData)
Method returns the calculated spectrum values with fall and rise times applied


### Notes
You can use this code to calculate the bin frequency like this
```
public float BinFrequency(ArrayData data, uint bin)
{
    if (data.FrequencyScale == ScaleType.Linear)
    {
        return bin * data.FrequencyStep + data.MinFrequency;
    }
    else
    {
        return data.MinFrequency * (float) Math.Pow(data.FrequencyStep, bin);
    }
}
```



You access data as you would in an array
    ArrayData a1;
    float third_value_channel_0 = a1[0][3];


## VisualizerDrawEventArgs
This is a class an instance of which is passed into the Draw event of the CustomVisualizer class
###Properties
#### Data (VisualizationDataFrame)
This property contains the visualization data, volume and spectrum. If this value is null then this means that there is no current data available - this could mean that the stream is stopped or after for example seek the audio analyzer has not yet caught up with buffering.
#### DrawingSession (Object)
This is the Win2D CanvasDrawingSession object you can use to draw the control. You need to cast it to CanvasDrawingSession first before using
`var drawingSession = (CanvasDrawingSession)args.DrawingSession;
#### PresentationTime (TimeSpan ?)
Represent the actual rendering position of the stream, you can use this to get the actual play position
#### ViewExtent (Size)
This is the size of the client area of the control. Use this to scale your drawing for the control when resized

# Build and Test
TODO: Describe and show how to build your code and run the tests. 

# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

If you want to learn more about creating good readme files then refer the following [guidelines](https://www.visualstudio.com/en-us/docs/git/create-a-readme). You can also seek inspiration from the below readme files:
- [ASP.NET Core](https://github.com/aspnet/Home)
- [Visual Studio Code](https://github.com/Microsoft/vscode)
- [Chakra Core](https://github.com/Microsoft/ChakraCore)
