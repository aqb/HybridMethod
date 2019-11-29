#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Kinect.h>

#include <iostream>
#include <string.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

using namespace std;
using namespace cv;

struct ColorJoint {
	cv::Scalar inputColor;
	cv::Scalar outputColor;
	cv::Point3f coord;
	cv::Point3f tempCoord;
	cv::Point2i centroid;
	int points;
};

typedef struct tagHSVTRIO {
	BYTE    hue;
	BYTE    saturation;
	BYTE    value;
} HSVTRIO;

vector<ColorJoint> colorJoint;

HRESULT hr;
IKinectSensor*           m_pKinectSensor;
ICoordinateMapper*		 m_pCoordinateMapper;
DepthSpacePoint*		 m_pDepthCoordinates;
IMultiSourceFrameReader* m_pMultiSourceFrameReader;
IBodyFrameReader*        m_pBodyFrameReader;
CameraSpacePoint*		 m_pCameraCoordinates;
ColorImageFormat		 m_imageFormat;
RGBQUAD*                 m_pColorRGBX;

int        cDepthWidth = 512;
int        cDepthHeight = 424;
int        cColorWidth = 1920;
int        cColorHeight = 1080;

int lh = 5, ls = 15, lv = 15;
int kernnel =32;

cv::Mat frame1, frame2, frame3, temp;
FILE *outFile;
bool startOutput = false;

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

//Marcar as cores ja selecionadas
cv::Scalar getColor(int index) {
	/*switch (index) {
		case 0: return cv::Scalar(0,0,0);
		case 1: return cv::Scalar(0, 255, 0);
		default: return cv::Scalar(255, 0, 0);
	}*/
	//Magente em BGR
	//return cv::Scalar(128,0,255);
	//Magente em HSV
	return cv::Scalar(150, 255, 255);

}

#define COLORDIFFERENCE 40
//Limiar
bool isSameColorHSV(int rh, int rs, int rv, int h, int s, int v) {
	/*if (abs(rr - r) < COLORDIFFERENCE && abs(rg - g) < COLORDIFFERENCE && abs(rb - b) < COLORDIFFERENCE) {
		return true;
	}*/
	// Limiar em RGB
	/*if ((abs(rr - r) + abs(rg - g) + abs(rb - b)) / (255.0f * 3) < 0.05) {
		return true;
	}*/
	
	//Limiar para HSV
	//rk = Hue, g = Saturation, b = valeu+
	//if (abs(rb - r) < 15 && abs(rg - g) < 50 && abs(rr - b) < 50)
	if (abs(rh - h) < 5 && abs(rs - s) < 33 && abs(rv - v) < 47)
	{
		return true;
	}

	/*if (rr > 0) {
		return true;
	}*/
	return false;
}

bool isSameColorBGR(int rb, int rg, int rr, int b, int g, int r) {
	/*if (abs(rr - r) < COLORDIFFERENCE && abs(rg - g) < COLORDIFFERENCE && abs(rb - b) < COLORDIFFERENCE) {
		return true;
	}*/
	// Limiar em RGB
	/*if ((abs(rr - r) + abs(rg - g) + abs(rb - b)) / (255.0f * 3) < 0.05) {
		return true;
	}*/
	//Precisa Ajustar limiar
	if (abs(rb - b) < 15 && abs(rg - g) < 15 && abs(rr - r) < 15)
	{
		return true;
	}

	/*if (rr > 0) {
		return true;
	}*/
	return false;
}
//Selecionar Cor

/*void LupaDeCor(int event, int x, int y, int flags, void* userdata) {
	if (event == cv::EVENT_RBUTTONDOWN) {
		cv::Vec4b color3 = frame3.at<cv::Vec4b>(y, x);
		printf("Cor RGB: %d %d %d\n", color3[2], color3[1], color3[0]);
	}
}*/

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_RBUTTONDOWN) {
		cv::Vec4b color3 = frame2.at<cv::Vec4b>(y, x);
		printf("Cor HSV: %d %d %d\n", color3[2], color3[1], color3[0]);
	}
	
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		
		
		//cv::Vec4b color1 = frame1.at<cv::Vec4b>(y, x);
		cv::Vec3b color2 = frame2.at<cv::Vec3b>(y, x);
		cv::Scalar outputColor = getColor(colorJoint.size());
		ColorJoint hd;

		//printf("%d %d %d -> %d %d %d\n", h, s, v, (int)color2.val[0], (int)color2.val[1], (int)color2.val[2]);

		hd.inputColor[0] = color2.val[0];
		hd.inputColor[1] = color2.val[1];
		hd.inputColor[2] = color2.val[2];
		hd.centroid.x = x;
		hd.centroid.y = y;
		printf("Adicionou a cor %d %d %d, em HSV, na lista de marcadores de cor.\n", color2[0], color2[1], color2[2]);
		// printf("Coordenadas do marcador: (%d,%d)\n", x, y);
		printf("Coordenadas do marcador: (%d,%d)\n", (int)hd.centroid.x, (int)hd.centroid.y);

		hd.outputColor = outputColor;

		colorJoint.push_back(hd);	

		/*outputColor = getColor(colorJoint.size());
		
		hd.inputColor[0] = color1.val[0];
		hd.inputColor[1] = color1.val[1];
		hd.inputColor[2] = color1.val[2];

		printf("Adicionou a cor %d %d %d, em RGB, na lista de marcadores de cor.\n", color1[2], color1[1], color1[0]);

		colorJoint.push_back(hd);*/

		printf("Quantidade de pontos selicionados: %d\n", colorJoint.size());

		
		
		//printf("adicionou a cor %d %d %d na lista de marcadores de cor.\n", outputColor[2], outputColor[1], outputColor[0]);
	}
	//else if (event == cv::EVENT_RBUTTONDOWN)
	//{
	//	//cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	//}
	//else if (event == cv::EVENT_MBUTTONDOWN)
	//{
	//	//cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	//}
	//else if (event == cv::EVENT_MOUSEMOVE)
	//{
	//	//cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
	//}
}

cv::Point2f BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height)
{
	// Calculate the body's position on the screen
	ColorSpacePoint colorPoint = { 0 };
	m_pCoordinateMapper->MapCameraPointToColorSpace(bodyPoint, &colorPoint);

	float screenPointX = static_cast<float>(colorPoint.X * width) / cColorWidth;
	float screenPointY = static_cast<float>(colorPoint.Y * height) / cColorHeight;

	return cv::Point2f(screenPointX, screenPointY);
}

cv::Point3f coord3d(0, 0, 0);

void updateMarkerPosition() {

	if (colorJoint.size() == 0) return;
	
	//printf("%d\n", colorJoint.size());

	cv::cvtColor(frame1, frame2, CV_BGRA2BGR);
	cv::cvtColor(frame2, frame2, CV_BGR2HSV);


	//RGBQUAD* pixelsBRG = (RGBQUAD*)frame3.data;
	HSVTRIO* pixelsHSV = (HSVTRIO*)frame2.data;
	//unsigned char *pixels = frame.data;

	coord3d = cv::Point3f(0,0,0);

	int count = 0;

	for (int j = 0; j < colorJoint.size(); j++) {
		ColorJoint &hd = colorJoint[j];
		hd.tempCoord = cv::Point3f(0, 0, 0);
		hd.points = 0;
	}

	for(int i = 0; i < colorJoint.size();i++)
	{
		ColorJoint &hd = colorJoint[i];
		int w = 0;
		//printf("Marcador: %d\n", i);
		for(int j = (hd.centroid.y - kernnel); j < (hd.centroid.y + kernnel); j++)
		{
			//printf("j : %d\t", j);
			for(int k = (hd.centroid.x - kernnel); k < (hd.centroid.x + kernnel); k++)
			{
				//printf("k : %d\t", k);
				w = j*cColorWidth + k;//conta de relação de dois dados para fazer um unico
				//printf("w : %d\n",w);
				if (isSameColorHSV(hd.inputColor[0], hd.inputColor[1], hd.inputColor[2], pixelsHSV[w].hue, pixelsHSV[w].saturation, pixelsHSV[w].value)) {
				//if (isSameColor(hd.inputColor[0], hd.inputColor[1], hd.inputColor[2], pixels[i*3+0], pixels[i * 3 + 1], pixels[i * 3 + 2])) {
					//printf("Encontrei a porra da cor!!!!!!\n");
					frame1.at<cv::Vec4b>(j, k) = cv::Vec4b(128, 0, 255, 0);
					//frame1.at<cv::Vec4b>(i / cColorWidth, i%cColorWidth) = cv::Vec4b(128, 0, 255, 0);
					//frame2.at<cv::Vec3b>(i / cColorWidth, i%cColorWidth) = cv::Vec3b(pixelsHSV[i].hue, pixelsHSV[i].saturation, pixelsHSV[i].value);
					if (m_pCameraCoordinates[w].X != -std::numeric_limits<float>::infinity()) {
						hd.tempCoord.x += m_pCameraCoordinates[w].X;
						hd.tempCoord.y += m_pCameraCoordinates[w].Y;
						hd.tempCoord.z += m_pCameraCoordinates[w].Z;
						hd.points++;
						//printf("Ponto Registrado!!!!!\n");
					}
				}
			}
		}
	}
	//Parte antiga de identificação de Cor, pixel a pixel into full scream
	/*for (int i = 0; i < cColorHeight *cColorWidth; i++) {
		
		for (int j = 0; j < colorJoint.size(); j++) {
			ColorJoint &hd = colorJoint[j];
			// Valores de entrada são RGB (na verdade é BGR) e HSV, a viavel hd é RGB precisa de conversão algebrica
			if (isSameColorHSV(hd.inputColor[0], hd.inputColor[1], hd.inputColor[2], pixelsHSV[i].hue, pixelsHSV[i].saturation, pixelsHSV[i].value)) {
				//if (isSameColor(hd.inputColor[0], hd.inputColor[1], hd.inputColor[2], pixels[i*3+0], pixels[i * 3 + 1], pixels[i * 3 + 2])) {
				frame1.at<cv::Vec4b>(i / cColorWidth, i%cColorWidth) = cv::Vec4b(128, 0, 255, 0);
				//frame2.at<cv::Vec3b>(i / cColorWidth, i%cColorWidth) = cv::Vec3b(pixelsHSV[i].hue, pixelsHSV[i].saturation, pixelsHSV[i].value);
				if (m_pCameraCoordinates[i].X != -std::numeric_limits<float>::infinity()) {
					hd.tempCoord.x += m_pCameraCoordinates[i].X;
					hd.tempCoord.y += m_pCameraCoordinates[i].Y;
					hd.tempCoord.z += m_pCameraCoordinates[i].Z;
					hd.points++;
				}
			}

		}
		
	}*/
	
	for (int j = 0; j < colorJoint.size(); j++) {
		ColorJoint &cj = colorJoint[j];
		if (cj.points > 0) {
			cj.coord.x = cj.tempCoord.x / (float)cj.points;
			cj.coord.y = cj.tempCoord.y / (float)cj.points;
			cj.coord.z = cj.tempCoord.z / (float)cj.points;
			//printf("Novo centroide: (%f,%f) e total e pontos %d\n", cj.coord.x, cj.coord.y,cj.points);
		}
		else {
			cj.coord.x = -std::numeric_limits<float>::infinity();
			cj.coord.y = -std::numeric_limits<float>::infinity();
			cj.coord.z = -std::numeric_limits<float>::infinity();
		}
		
	}

	for (int j = 0; j < colorJoint.size(); j++) {
		//printf("Devia esta pintando o centroide \n");
		CameraSpacePoint ctemp;
		ctemp.X = colorJoint[j].coord.x;
		ctemp.Y = colorJoint[j].coord.y;
		ctemp.Z = colorJoint[j].coord.z;
		cv::Point2f algc = BodyToScreen(ctemp, cColorWidth, cColorHeight);
		if (algc.x == -std::numeric_limits<float>::infinity())
		{
			colorJoint[j].centroid.x = colorJoint[j].centroid.x;
		}
		else
		{
			colorJoint[j].centroid.x = (int)algc.x;
		}
		if (algc.y == -std::numeric_limits<float>::infinity())
		{
			colorJoint[j].centroid.y = colorJoint[j].centroid.y;
		}
		else
		{	
			colorJoint[j].centroid.y = (int)algc.y;
		}
		
		cv::circle(frame1, algc, 6, cv::Scalar(0, 0, 255), -1); //era 5 e -1
		cv::circle(frame1, algc, 4, cv::Scalar(255, 255, 255), -1);
		cv::circle(frame1, algc, 2, cv::Scalar(0, 0, 255), -1);
		// printf("%d -- (%d %d)\n", j, colorJoint[j].centroid.x, colorJoint[j].centroid.y);

	}
	/*for (int j = 1; j < colorJoint.size(); j+=2) {
		//printf("Devia esta pintando o centroide \n");
		CameraSpacePoint ctemp;
		ctemp.X = colorJoint[j].coord.x;
		ctemp.Y = colorJoint[j].coord.y;
		ctemp.Z = colorJoint[j].coord.z;
		cv::Point2f algc = BodyToScreen(ctemp, cColorWidth, cColorHeight);
	
		cv::circle(frame3, algc, 10, cv::Scalar(255, 0, 0), -1); //era 5 e -1
		cv::circle(frame3, algc, 5, cv::Scalar(255, 255, 255), -1);
		cv::circle(frame3, algc, 2, cv::Scalar(255, 0, 0), -1);
		//printf("%f %f", algc.x, algc.y);

	}*/
}

void main() {

	outFile = fopen("output.txt", "w");
	
	cv::namedWindow("Pontos HSV", 1);
	//cv::namedWindow("Pontos RGB", 1);

	//cv::setMouseCallback("Pontos RGB", CallBackFunc, NULL);
	cv::setMouseCallback("Pontos HSV", CallBackFunc, NULL);
	
	
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	hr = m_pKinectSensor->Open();
	hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
	hr = m_pKinectSensor->OpenMultiSourceFrameReader(FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color, &m_pMultiSourceFrameReader);
	
	IBodyFrameSource* pBodyFrameSource = NULL;
	hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
	hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);

	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
	m_pCameraCoordinates = new CameraSpacePoint[cColorHeight*cColorWidth];

	frame1.create(cColorHeight, cColorWidth, CV_8UC4);
	frame3.create(cColorHeight, cColorWidth, CV_8UC4);
	frame2.create(cColorHeight, cColorWidth, CV_8UC3);

	while (true) {

		//Update();
		IMultiSourceFrame* pMultiSourceFrame = NULL;
		IDepthFrame* pDepthFrame = NULL;
		IColorFrame* pColorFrame = NULL;
		IBodyFrame* pBodyFrame = NULL;

		hr = m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);
		if (hr == S_OK) {
			IDepthFrameReference* pDepthFrameReference = NULL;
			IColorFrameReference* pColorFrameReference = NULL;
			hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
			
			hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
			if (hr == S_OK) {
				INT64 nDepthTime = 0;
				IFrameDescription* pDepthFrameDescription = NULL;
				int nDepthWidth = 0;
				int nDepthHeight = 0;
				UINT nDepthBufferSize = 0;
				UINT16 *pDepthBuffer = NULL;

				hr = pDepthFrame->get_RelativeTime(&nDepthTime);
				hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);
				hr = pDepthFrameDescription->get_Width(&nDepthWidth);
				hr = pDepthFrameDescription->get_Height(&nDepthHeight);
				hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);

				hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
				hr = pColorFrameReference->AcquireFrame(&pColorFrame);
				if (hr == S_OK) {
					IFrameDescription* pColorFrameDescription = NULL;
					int nColorWidth = 0;
					int nColorHeight = 0;
					m_imageFormat = ColorImageFormat_None;
					UINT nColorBufferSize = 0;
					RGBQUAD *pColorBuffer = NULL;

					hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
					hr = pColorFrameDescription->get_Width(&nColorWidth);
					hr = pColorFrameDescription->get_Height(&nColorHeight);
					hr = pColorFrame->get_RawColorImageFormat(&m_imageFormat);

					if (m_imageFormat == ColorImageFormat_Bgra)
					{
						hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
					}
					else if (m_pColorRGBX)
					{
						pColorBuffer = m_pColorRGBX;
						nColorBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
						hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
					}
					else
					{
						hr = E_FAIL;
					}

					hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, m_pDepthCoordinates);
					hr = m_pCoordinateMapper->MapColorFrameToCameraSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, m_pCameraCoordinates);

					//cv::cvtColor(frame, frame, CV_HSV2BGR);
					//cv::cvtColor(frame, frame, CV_BGR2BGRA);

					memcpy(frame1.data, pColorBuffer, nColorBufferSize);
					memcpy(frame3.data, pColorBuffer, nColorBufferSize);
					cv::cvtColor(frame1, temp, CV_BGRA2BGR);
					cv::cvtColor(temp, frame2, CV_BGR2HSV);

					updateMarkerPosition();

				}

			}
			SafeRelease(pDepthFrameReference);
			SafeRelease(pColorFrameReference);

			/*IFrameDescription* pBodyIndexFrameDescription = NULL;
			int nBodyIndexWidth = 0;
			int nBodyIndexHeight = 0;
			UINT nBodyIndexBufferSize = 0;
			BYTE *pBodyIndexBuffer = NULL;*/
			
			
		}

		hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
		if (hr == S_OK) {
			IBody* ppBodies[BODY_COUNT] = { 0 };
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
			for (int i = 0; i < BODY_COUNT; ++i)
			{
				IBody* pBody = ppBodies[i];
				if (pBody)
				{
					BOOLEAN bTracked = false;
					hr = pBody->get_IsTracked(&bTracked);

					if (SUCCEEDED(hr) && bTracked)
					{
						Joint joints[JointType_Count];
						cv::Point2f jointPoints[JointType_Count];
						//HandState leftHandState = HandState_Unknown;
						//HandState rightHandState = HandState_Unknown;

						//pBody->get_HandLeftState(&leftHandState);
						//pBody->get_HandRightState(&rightHandState);

						hr = pBody->GetJoints(_countof(joints), joints);
						if (SUCCEEDED(hr))
						{
							for (int j = 0; j < _countof(joints); ++j)
							{
								jointPoints[j] = BodyToScreen(joints[j].Position, cColorWidth, cColorHeight);
								cv::circle(frame1, jointPoints[j], 5, cv::Scalar(0, 255, 0), -1);
							}

							/*for (int j = 0; j < colorJoint.size(); ++j)
							{
								CameraSpacePoint csp;
								csp.X = colorJoint[j].coord.x;
								csp.Y = colorJoint[j].coord.y;
								csp.Z = colorJoint[j].coord.z;
								cv::Point2f p2d = BodyToScreen(csp, cColorWidth, cColorHeight);
								cv::circle(frame1, p2d, 5, cv::Scalar(0, 0, 255), -1);
								//printf(".");
							}*/
							
							if (startOutput) {

								for (int j = 0; j < _countof(joints); ++j) {
									if (joints[i].TrackingState == TrackingState_NotTracked) {
										fprintf(outFile, "%f %f %f ", -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
									}
									else {
										fprintf(outFile, "%f %f %f ", joints[j].Position.X, joints[j].Position.Y, joints[j].Position.Z);
									}
								}

								for (int j = 0; j < colorJoint.size(); j++) {
									ColorJoint hd = colorJoint[j];
									fprintf(outFile, "%f %f %f ", hd.coord.x, hd.coord.y, hd.coord.z);
								}

								fprintf(outFile, "\n");

							}

							//DrawBody(joints, jointPoints);

							//DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
							//DrawHand(rightHandState, jointPoints[JointType_HandRight]);
						}
					}
				}
			}
		}
	
		cv::imshow("Pontos HSV", frame1);
		//cv::imshow("Pontos RGB", frame3);

		//imshow("test2", frame2);
		int c = cv::waitKey(10);
		if (c == 'c') {
			startOutput = true;
			string filenameS;
			cout << "Entre com o nome do arquivo: ";
				getline(cin,filenameS);
				const char *filename = filenameS.c_str();;
			outFile = fopen(filename, "w");
			printf("\n Gravando\n");
		} else if (c == 'r') {
			printf("\\\\\\\\\\\\\\\\\\\\\\\System Reset///////////////////\n*\n*\n");
			colorJoint.clear();
		}
		else if (c == 's') {
			startOutput = false;
			fclose(outFile);
			cin.clear();
			printf("\n Gravacao parada\nx*\n*\n");
		}
/*		else if (c == 'a') {
			printf("\n Entre com os novos limites:\nLIMITE H: ");
			scanf("%i", lh);
			printf("\nLIMITE S: ");
			scanf("%i", ls);
			printf("\nLIMITE V: ");
			scanf("%i", lv);
		}*/

		SafeRelease(pDepthFrame);
		SafeRelease(pColorFrame);
		SafeRelease(pBodyFrame);
		SafeRelease(pMultiSourceFrame);

	}
}