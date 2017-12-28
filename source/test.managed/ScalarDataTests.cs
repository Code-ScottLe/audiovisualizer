﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using AudioVisualizer;

namespace test.managed
{
    [TestClass]
    public class ScalarDataTests
    {
        [TestCategory("ScalarData")]
        [TestMethod]
        public void ScalarData_CreateEmpty()
        {
            var data = ScalarData.CreateEmpty(2);
            Assert.AreEqual(2, data.Count());
            Assert.AreEqual(2, data.Count);
            Assert.AreEqual(ScaleType.Linear, data.AmplitudeScale);
            Assert.AreEqual(0.0f,data[0]);
            Assert.AreEqual(0.0f, data[1]);
            CollectionAssert.AreEqual(new float[] { 0.0f, 0.0f }, data.AsEnumerable().ToArray());
        }

        [TestCategory("ScalarData")]
        [TestMethod]
        public void ScalarData_CreateWithValues()
        {
            var initData = new float[] { 0.0f, 0.1f, 0.2f, 0.3f, 0.4f };
            var data = ScalarData.Create(initData);
            Assert.AreEqual(5, data.Count());
            Assert.AreEqual(5, data.Count);
            Assert.AreEqual(ScaleType.Linear, data.AmplitudeScale);
            for (int i = 0; i < 5; i++)
            {
                Assert.AreEqual(initData[i], data[i]);
            }
        }

        [TestCategory("ScalarData")]
        [TestMethod()]
        public void ScalarData_ConvertToDecibels()
        {
            float[] testValues = new float [] { 0.0f, 0.1f, 1.0f, 1e-6f, 1e6f, -1 };
            var data = ScalarData.Create(testValues);
            var logData = data.ConvertToDecibels(-100, 0);
            CollectionAssert.AreEqual(new float[] { -100.0f, -20.0f, 0.0f, -100.0f, 0.0f, -100.0f }, logData.ToArray());
            Assert.ThrowsException<Exception>(() => { var d2 = logData.ConvertToDecibels(-100, 0); });
            Assert.ThrowsException<ArgumentException>(() => { var d3 = data.ConvertToDecibels(0, 0); });
        }


        [TestCategory("ScalarData")]
        [TestMethod()]
        public void ScalarData_RiseAndFall()
        {
            var data = ScalarData.Create(new float[] { 1.0f, 2.0f, 1.5f }); // First falling, second rising, 3rd same
            var previous = ScalarData.Create(new float[] { 2.0f, 1.0f, 1.5f });
            var result = data.ApplyRiseAndFall(previous,
                TimeSpan.FromMilliseconds(100),
                TimeSpan.FromMilliseconds(200),
                TimeSpan.FromMilliseconds(400)
                );
            CollectionAssert.AreEqual(
                new float[] { 1.135336f, 1.98168433f, 1.5f }, result.ToArray());

            var result2 = data.ApplyRiseAndFall(null,
                TimeSpan.FromMilliseconds(100),
                TimeSpan.FromMilliseconds(200),
                TimeSpan.FromMilliseconds(400));

            CollectionAssert.AreEqual(
                new float[] { 0.9816843f, 1.96336865f, 1.47252655f }, result2.ToArray());

            Assert.ThrowsException<ArgumentException>(
                ()=> {
                    data.ConvertToDecibels(-100, 20).ApplyRiseAndFall(previous, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1));
                }
                );
            Assert.ThrowsException<ArgumentException>(
                () => {
                    data.ApplyRiseAndFall(previous, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1), TimeSpan.Zero);
                }
                );
            var data2 = ScalarData.CreateEmpty(2);  // Different size
            Assert.ThrowsException<ArgumentException>(
                () =>
                {
                    data2.ApplyRiseAndFall(previous, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1));
                }
                );
        }
        [TestCategory("ScalarData")]
        [TestMethod()]
        public void ScalarData_CombineChannels()
        {
            ScalarData data = ScalarData.Create(new float[] { 1, 2, 3, 4, 5 });

            Assert.ThrowsException<NullReferenceException>(() => { data.CombineChannels(null, 2); });
            Assert.ThrowsException<ArgumentException>(() => { data.CombineChannels(new float[9], 2); });
            float[] map5to2 = new float[] { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 10f, 20f, 30f, 40f, 50f };
            var newData = data.CombineChannels(map5to2, 2);
            float[] expected = new float[] { 5.5f, 550f };
            CollectionAssert.AreEqual(expected, newData.ToArray());

            ScalarData monoData = ScalarData.Create(new float[] { 1 });
            float[] map = new float[] { 0.1f, 0.2f,0.3f };
            var data3ch = monoData.CombineChannels(map, 3);
            CollectionAssert.AreEqual(map, data3ch.ToArray());

            ScalarData logData = data.ConvertToDecibels(-100, 20);
            Assert.ThrowsException<Exception>(()=> { logData.CombineChannels(map5to2, 2); });
        }

    }

}
