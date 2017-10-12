using AudioVisualizer;
using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.Text;
using System;
using System.Numerics;
using Windows.Media.Core;
using Windows.Media.Playback;
using Windows.Storage.Pickers;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409




namespace VisualizationPlayer
{
    // In Visual Studio:
    // Tools-> Package Manager Settings
    // Select Package Sources
    // Click green add button
    // Name: Audivizualizer MyGet
    // Source: https://www.myget.org/F/uwpaudiovisualizer/api/v3/index.json


    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        #region Bootstrap
        MediaPlayer _player;

        public MainPage()
        {
            this.InitializeComponent();
            _player = new MediaPlayer();
            _player.MediaOpened += Player_MediaOpened;
            _player.PlaybackSession.PositionChanged += PlaybackSession_PositionChanged;
        }

        bool _bInsideUpdate = false;
        private async void PlaybackSession_PositionChanged(MediaPlaybackSession sender, object args)
        {

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal,
    () =>
    {
        _bInsideUpdate = true;
        seekBar.Value = _player.PlaybackSession.Position.TotalSeconds;
        _bInsideUpdate = false;
    }
    );
        }

        private async void Player_MediaOpened(MediaPlayer sender, object args)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal,
                () =>
                {
                    seekBar.Maximum = _player.PlaybackSession.NaturalDuration.TotalSeconds;
                }
            );
        }

        private async void OpenFile_Click(object sender, RoutedEventArgs e)
        {
            var picker = new FileOpenPicker();
            picker.FileTypeFilter.Add(".wav");
            picker.FileTypeFilter.Add(".mp3");
            picker.FileTypeFilter.Add(".wmv");
            picker.FileTypeFilter.Add(".mp4");
            picker.FileTypeFilter.Add(".flac");
            var file = await picker.PickSingleFileAsync();
            if (file != null)
            {
                _player.Source = MediaSource.CreateFromStorageFile(file);
                _player.Play();
            }
        }


        private IVisualizationSource m_VisualizationSource;

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            CreateVisualizer();
        }

        private async void CreateVisualizer()
        {

            m_VisualizationSource = await AudioVisualizer.VisualizationSource.CreateFromMediaPlayerAsync(_player);
            visualizer.Source = m_VisualizationSource;
        } 
        #endregion

        ScalarData _emptyVolumeData = new ScalarData(2);    // Create empty data for volume data

        ArrayData _emptySpectrum = new ArrayData(2, 20);
        ArrayData _previousSpectrum;
        ArrayData _previousPeakSpectrum;

        TimeSpan _rmsRiseTime = TimeSpan.FromMilliseconds(50);
        TimeSpan _rmsFallTime = TimeSpan.FromMilliseconds(50);
        TimeSpan _peakRiseTime = TimeSpan.FromMilliseconds(10);
        TimeSpan _peakFallTime = TimeSpan.FromMilliseconds(3000);
        TimeSpan _frameDuration = TimeSpan.FromMilliseconds(16.7);

        private void visualizer_Draw(AudioVisualizer.IVisualizer sender, AudioVisualizer.VisualizerDrawEventArgs args)
        {
            var drawingSession = (CanvasDrawingSession)args.DrawingSession;

            CanvasTextFormat textFormat = new CanvasTextFormat();
            textFormat.VerticalAlignment = CanvasVerticalAlignment.Center;
            textFormat.HorizontalAlignment = CanvasHorizontalAlignment.Center;
            textFormat.FontSize = 9;

            var spectrum = args.Data != null ? args.Data.Spectrum.TransformLinearFrequency(20) : _emptySpectrum;

            _previousSpectrum = spectrum.ApplyRiseAndFall(_previousSpectrum, _rmsRiseTime, _rmsFallTime, _frameDuration);
            _previousPeakSpectrum = spectrum.ApplyRiseAndFall(_previousPeakSpectrum, _peakRiseTime, _peakFallTime, _frameDuration);

            var logSpectrum = _previousSpectrum.ConvertToLogAmplitude(-50, 0);
            var logPeakSpectrum = _previousPeakSpectrum.ConvertToLogAmplitude(-50, 0);

            Vector2 prevPointLeft = new Vector2(), prevPointRight = new Vector2();

            for (int i = 0; i < logSpectrum.FrequencyCount; i++)
            {
                float barHeight0 = 160 + 3.2f * logSpectrum[0][i];
                drawingSession.FillRectangle(i * 50, 320 - barHeight0, 50, barHeight0, Colors.WhiteSmoke);
                float barHeight1 = 160 + 3.2f * logSpectrum[1][i];
                drawingSession.FillRectangle(i * 50, 340, 50, barHeight1, Colors.WhiteSmoke);

                Vector2 leftPoint = new Vector2(i * 50 + 25, 160 - 3.2f * logPeakSpectrum[0][i]);
                Vector2 rightPoint = new Vector2(i * 50 + 25, 500 + 3.2f * logPeakSpectrum[1][i]);
                if (i != 0)
                {
                    drawingSession.DrawLine(prevPointLeft, leftPoint, Colors.Red, 3);
                    drawingSession.DrawLine(prevPointRight, rightPoint, Colors.Red, 3);
                }
                prevPointLeft = leftPoint;
                prevPointRight = rightPoint;

                string freqText = $"{Math.Round(logSpectrum.FrequencyStep * i * 1e-3),2:F0}k";
                drawingSession.DrawText(freqText, i * 50 + 25, 330, Colors.White, textFormat);
            }
        }

        private void seekBar_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (!_bInsideUpdate)
            {
                _player.PlaybackSession.Position = TimeSpan.FromSeconds(e.NewValue);
            }
        }
    }
}
