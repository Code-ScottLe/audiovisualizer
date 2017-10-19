#include "pch.h"
#include "CppUnitTest.h"
#include <math.h>
#include <limits>
#include <DirectXMath.h>
#include <AudioMath.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace AudioVisualizer::Math;

namespace AnalyzerTest
{
	TEST_CLASS(MathTests)
	{
	public:
		TEST_METHOD(Math_RiseAndFall)
		{
			using namespace DirectX;
			XMVECTOR vPrevious = XMVectorSet(1, 1, 2, 1);
			XMVECTOR vCurrent = XMVectorSet(1, 2, 1, 11);
			XMVECTOR vResult = XMVectorReplicate(std::numeric_limits<float>::quiet_NaN());
			ApplyRiseAndFall(&vPrevious, &vCurrent, &vResult, 1, 1, 3);

			Assert::AreEqual(1, XMVectorGetByIndex(vResult,0), 0.001f, L"1->1");
			Assert::AreEqual(1.63212055f, XMVectorGetByIndex(vResult, 1), 0.001f, L"1->2");
			Assert::AreEqual(1.04978707f, XMVectorGetByIndex(vResult, 2), 0.001f, L"2->1");
			Assert::AreEqual(7.3212055f, XMVectorGetByIndex(vResult, 3), 0.001f, L"1->11");

			XMVECTOR vResult2 = XMVectorReplicate(std::numeric_limits<float>::quiet_NaN());
			ApplyRiseAndFall(nullptr, &vResult, &vResult2, 1, 1, 3);

			Assert::AreEqual(0.63212049f, XMVectorGetByIndex(vResult2, 0), 0.001f);
			Assert::AreEqual(1.0316968f, XMVectorGetByIndex(vResult2, 1), 0.001f);
			Assert::AreEqual(0.663591921f, XMVectorGetByIndex(vResult2, 2), 0.001f);
			Assert::AreEqual(4.62788391f, XMVectorGetByIndex(vResult2, 3), 0.001f);
		}
		TEST_METHOD(Math_ConvertToLog)
		{
			using namespace DirectX;
			XMVECTOR vInput[2] = { XMVectorSet(-1.0f,0.0f,1e-6f,0.1f),XMVectorSet(1.0f,10.0f,std::numeric_limits<float>::max(),INFINITY) };
			XMVECTOR vResult[2];
			ConvertToLogarithmic(vInput, vResult, 2, -100.0f, 20.0f);
			float expected[] = { -100,-100,-100,-20,0,20,20,20 };
			for (size_t i = 0; i < 8; i++)
			{
				Assert::AreEqual(expected[i], ((float*)vResult)[i]);
			}
		}

		TEST_METHOD(Math_SpectrumLinearTransform)
		{
			float testData[10] = { 0,1,2,3,4,5,6,7,8,9 };
			float output1[4] = { -100,-100,-100,-100 };
			float expected[4] = { 2, 8 , 14.5 , 20.5 };
			SpectrumLinearTransform(testData, 10, output1, 4);
			for (size_t i = 0; i < 4; i++)
			{
				Assert::AreEqual(expected[i], output1[i]);
			}

			float testData2[2] = { 0,1 };
			float output2[4] = { std::numeric_limits<float>::quiet_NaN(),
								 std::numeric_limits<float>::quiet_NaN(),
								 std::numeric_limits<float>::quiet_NaN(),
								 std::numeric_limits<float>::quiet_NaN()};

		}
	};
}