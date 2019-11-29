//#include <Kinect.h>
//
//#define WIN32_LEAN_AND_MEAN
//
//#include <windows.h>
//
//#include <stdio.h>
//
//#include <opencv2/highgui/highgui.hpp>
//
//IKinectSensor*          m_pKinectSensor;
//IColorFrameReader*      m_pColorFrameReader;
//IDepthFrameReader*      m_pDepthFrameReader;
//RGBQUAD*                m_pColorRGBX;
//RGBQUAD*                m_pDepthRGBX;
//
//int cColorWidth = 1920;
//int cColorHeight = 1080;
//cv::Mat colorFrame;
//cv::Mat depthFrame;
//
//template<class Interface>
//inline void SafeRelease(Interface *& pInterfaceToRelease)
//{
//	if (pInterfaceToRelease != NULL)
//	{
//		pInterfaceToRelease->Release();
//		pInterfaceToRelease = NULL;
//	}
//}
//
//HRESULT InitializeDefaultSensor()
//{
//	HRESULT hr;
//
//	hr = GetDefaultKinectSensor(&m_pKinectSensor);
//	if (FAILED(hr))
//	{
//		return hr;
//	}
//
//	if (m_pKinectSensor)
//	{
//		// Initialize the Kinect and get the color reader
//		IColorFrameSource* pColorFrameSource = NULL;
//		IDepthFrameSource* pDepthFrameSource = NULL;
//
//		hr = m_pKinectSensor->Open();
//
//		if (SUCCEEDED(hr))
//		{
//			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
//		}
//
//		SafeRelease(pColorFrameSource);
//		SafeRelease(pDepthFrameSource);
//	}
//
//	if (!m_pKinectSensor || FAILED(hr))
//	{
//		printf("No ready Kinect found!\n");
//		return E_FAIL;
//	}
//
//	return hr;
//}
//
//void ProcessColor(INT64 nTime, RGBQUAD* pBuffer, int nWidth, int nHeight)
//{
//	// Make sure we've received valid data
//	if (pBuffer && (nWidth == cColorWidth) && (nHeight == cColorHeight))
//	{
//		memcpy(colorFrame.data, pBuffer, nWidth * nHeight * 4);
//		cv::imshow("test", colorFrame);
//		cv::waitKey(10);
//	}
//}
//
//void ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
//{
//	
//	// Make sure we've received valid data
//	//if (m_pDepthRGBX && pBuffer && (nWidth == cDepthWidth) && (nHeight == cDepthHeight))
//	{
//		RGBQUAD* pRGBX = m_pDepthRGBX;
//
//		// end pixel is start + width*height - 1
//		const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);
//
//		while (pBuffer < pBufferEnd)
//		{
//			USHORT depth = *pBuffer;
//
//			// To convert to a byte, we're discarding the most-significant
//			// rather than least-significant bits.
//			// We're preserving detail, although the intensity will "wrap."
//			// Values outside the reliable depth range are mapped to 0 (black).
//
//			// Note: Using conditionals in this loop could degrade performance.
//			// Consider using a lookup table instead when writing production code.
//			BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);
//
//			pRGBX->rgbRed = intensity;
//			pRGBX->rgbGreen = intensity;
//			pRGBX->rgbBlue = intensity;
//
//			++pRGBX;
//			++pBuffer;
//		}
//
//	}
//}
//
//void main() {
//
//	colorFrame.create(1080, 1920, CV_8UC4);
//
//	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
//	m_pDepthRGBX = new RGBQUAD[512 * 424];
//
//	InitializeDefaultSensor();
//
//	UINT16 *pBuffer = NULL;
//
//	while (true) {
//		IColorFrame* pColorFrame = NULL;
//		IDepthFrame* pDepthFrame = NULL;
//
//		HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
//
//		INT64 nTime = 0;
//		IFrameDescription* pFrameDescription = NULL;
//		ColorImageFormat imageFormat = ColorImageFormat_None;
//		UINT nBufferSize = 0;
//		RGBQUAD *pBuffer = NULL;
//		UINT16* pBufferDepth = NULL;
//
//		if (SUCCEEDED(hr))
//		{
//			int nWidthColor = 0;
//			int nHeightColor = 0;
//			
//
//			hr = pColorFrame->get_RelativeTime(&nTime);
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pColorFrame->get_FrameDescription(&pFrameDescription);
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pFrameDescription->get_Width(&nWidthColor);
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pFrameDescription->get_Height(&nHeightColor);
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				if (imageFormat == ColorImageFormat_Bgra)
//				{
//					hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
//				}
//				else if (m_pColorRGBX)
//				{
//					pBuffer = m_pColorRGBX;
//					nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
//					hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
//				}
//				else
//				{
//					hr = E_FAIL;
//				}
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				ProcessColor(nTime, pBuffer, nWidthColor, nHeightColor);
//			}
//
//			SafeRelease(pFrameDescription);
//		}
//
//		hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
//
//		if (SUCCEEDED(hr)) {
//
//			int nWidthDepth = 0;
//			int nHeightDepth = 0;
//
//
//			hr = pDepthFrame->get_RelativeTime(&nTime);
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pFrameDescription->get_Width(&nWidthDepth);
//			}
//
//			if (SUCCEEDED(hr))
//			{
//				hr = pFrameDescription->get_Height(&nHeightDepth);
//			}
//
//			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBufferDepth);
//
//			ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance);
//		}
//
//		SafeRelease(pColorFrame);
//
//
//	}
//
//}

#define RA

#include <KinectController.h>



void main() {
	KinectCamera *kinectCamera = new KinectCamera();

}