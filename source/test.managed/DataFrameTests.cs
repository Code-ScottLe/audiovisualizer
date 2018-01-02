using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using AudioVisualizer;

namespace test.managed
{
    [TestClass()]
    public class DataFrameTests
    {
        [TestCategory("VisualizationDataFrame")]
        [TestMethod()]
        public void DataFrame_ctor()
        {
            TimeSpan time = TimeSpan.FromSeconds(3);
            TimeSpan duration = TimeSpan.FromMilliseconds(16.7);
            ScalarData rms = ScalarData.CreateEmpty(2);
            ScalarData peak = ScalarData.CreateEmpty(2);
            SpectrumData spectrum = SpectrumData.CreateEmpty(2, 100, ScaleType.Linear, ScaleType.Linear, 0, 24000);
            VisualizationDataFrame frame = 
                new VisualizationDataFrame(
                    time,
                    duration,
                    rms,
                    peak,
                    spectrum
                    );

            Assert.IsNotNull(frame);
            Assert.AreEqual(time, frame.Time);
            Assert.AreEqual(duration, frame.Duration);
            Assert.AreSame(rms, frame.RMS);
            Assert.AreSame(peak, frame.Peak);
            Assert.AreSame(spectrum, frame.Spectrum);
        }

        [TestCategory("VisualizationDataFrame")]
        [TestMethod()]
        public void DataFrame_CombineChannels()
        {
            TimeSpan time = TimeSpan.FromSeconds(3);
            TimeSpan duration = TimeSpan.FromMilliseconds(16.7);
            ScalarData rms = ScalarData.CreateEmpty(5);
            ScalarData peak = ScalarData.CreateEmpty(5);
            SpectrumData spectrum = SpectrumData.CreateEmpty(5, 100, ScaleType.Linear, ScaleType.Linear, 0, 24000);
            VisualizationDataFrame frame =
                new VisualizationDataFrame(
                    time,
                    duration,
                    rms,
                    peak,
                    spectrum
                    );
            var newFrame = frame.CombineChannels(new float[10], 2); // We are not testing the conversion

            Assert.IsNotNull(newFrame);
            Assert.IsNotNull(newFrame.RMS);
            Assert.IsNotNull(newFrame.Peak);
            Assert.IsNotNull(newFrame.Spectrum);
            Assert.AreEqual(frame.Time, newFrame.Time);
            Assert.AreEqual(frame.Duration, newFrame.Duration);
            Assert.AreEqual(2, newFrame.RMS.Count,"newFrame RMS channelCount");
            Assert.AreEqual(2, newFrame.Peak.Count,"newFrame Peak channelCount");
            Assert.AreEqual(2, newFrame.Spectrum.Count, "newFrame Spectrum channelCount");
            Assert.AreEqual(frame.Spectrum.AmplitudeScale, newFrame.Spectrum.AmplitudeScale);
            Assert.AreEqual(frame.Spectrum.FrequencyCount, newFrame.Spectrum.FrequencyCount);
            Assert.AreEqual(frame.Spectrum.FrequencyScale, newFrame.Spectrum.FrequencyScale);
            Assert.AreEqual(frame.Spectrum.FrequencyStep, newFrame.Spectrum.FrequencyStep);
            Assert.AreEqual(frame.Spectrum.MaxFrequency, newFrame.Spectrum.MaxFrequency);
            Assert.AreEqual(frame.Spectrum.MinFrequency, newFrame.Spectrum.MinFrequency);

        }
    }
}
