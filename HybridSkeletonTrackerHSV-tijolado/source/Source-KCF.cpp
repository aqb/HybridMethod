#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Kinect.h>

#include <iostream>
#include <string.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/tracking/tracking.hpp>

#include <vector>

using namespace std;
using namespace cv;


//Availables tracking types 
vector<string> trackerTypes = { "BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT" };

// Specify the tracker type
string trackerType = "KCF";

// create tracker by name
Ptr<Tracker> createTrackerByName(string trackerType)
{	Ptr<Tracker> tracker;
	if (trackerType == trackerTypes[0])
		tracker = TrackerBoosting::create();

	else if (trackerType == trackerTypes[1])
		tracker = TrackerMIL::create();

	else if (trackerType == trackerTypes[2])
		tracker = TrackerKCF::create();

	else if (trackerType == trackerTypes[3])
		tracker = TrackerTLD::create();

	else if (trackerType == trackerTypes[4])
		tracker = TrackerMedianFlow::create();

	else if (trackerType == trackerTypes[5])
		tracker = TrackerGOTURN::create();

	else if (trackerType == trackerTypes[6])
		tracker = TrackerMOSSE::create();

	else if (trackerType == trackerTypes[7])
		tracker = TrackerCSRT::create();

	else {
		cout << "Incorrect tracker name" << endl;
		cout << "Available trackers are: " << endl;
		for (vector<string>::iterator it = trackerTypes.begin(); it != trackerTypes.end(); ++it)
			std::cout << " " << *it << endl;
	}
	
	return tracker;
}


// Create multitracker
//Ptr<MultiTracker> jointsTracker = cv::MultiTracker::create();
//Ptr<Tracker> jointTracker;// = TrackerKCF::create();


// Color Joints Struct definition
struct ColorJoint {
	cv::Scalar inputColor;
	cv::Scalar medColor;
	cv::Scalar outputColor;
	cv::Point3f coord;
	cv::Point3f tempCoord;
	cv::Point2d centroid;
	cv::Rect2d position;
	int points;
	Ptr<Tracker> jointTracker;
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
int kernnel =63;
int kernnelH = (kernnel / 2) + 1;

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



void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_RBUTTONDOWN) {
		cv::Vec3b color2 = frame1.at<cv::Vec3b>(y, x);
		printf("Cor RBG: %d %d %d\n", color2[0], color2[1], color2[2]);
		printf("Coordenadas do marcador: (%d,%d)\n", x, y);
	}
	
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		
		
		cv::Vec4b color1 = frame1.at<cv::Vec4b>(y, x);
		//cv::Vec3b color2 = frame2.at<cv::Vec3b>(y, x);
		cv::Scalar outputColor = getColor(colorJoint.size());
		ColorJoint hd;

		//printf("%d %d %d -> %d %d %d\n", h, s, v, (int)color2.val[0], (int)color2.val[1], (int)color2.val[2]);

		hd.inputColor[0] = color1.val[0];
		hd.medColor.val[0] = color1.val[0];
		hd.inputColor[1] = color1.val[1];
		hd.medColor.val[1] = color1.val[1];
		hd.inputColor[2] = color1.val[2];
		hd.medColor.val[2] = color1.val[2];
		hd.centroid.x = x;
		hd.centroid.y = y;
				
		//hd.jointTracker = TrackerKCF::create();

		//{ "BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT" };
		hd.jointTracker = createTrackerByName(trackerTypes[4]);

		hd.position = Rect2d((hd.centroid.x - kernnelH), (hd.centroid.y - kernnelH), kernnel, kernnel);
		rectangle(frame1, hd.position, hd.inputColor, 2, 1);
		
		std::cout << trackerTypes[4] << endl;


		hd.jointTracker->init(frame1,hd.position);
		
		if (hd.jointTracker == NULL)
		{
			std::cout << "***Error in the instantiation of the tracker...***\n";
		}
		else cout << "Successful Add Track" << endl;

	    //bool Add = jointsTracker->add(TrackerKCF::create(), frame1, hd.position);



		//printf("Adicionou a cor %d %d %d, em HSV, na lista de marcadores de cor.\n", color1[0], color1[1], color1[2]);
		printf("Coordenadas do marcador: (%d,%d)\n", (int)hd.centroid.x, (int)hd.centroid.y);

		hd.outputColor = outputColor;

		colorJoint.push_back(hd);	


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

//cv::Point3f coord3d(0, 0, 0);

void updateMarkerPosition()
{

	if (colorJoint.size() == 0) return;

	//printf("%d\n", colorJoint.size());

	//cv::cvtColor(frame1, frame2, CV_BGRA2BGR);
	//cv::cvtColor(frame2, frame2, CV_BGR2HSV);


	//RGBQUAD* pixelsBRG = (RGBQUAD*)frame1.data;
	//HSVTRIO* pixelsHSV = (HSVTRIO*)frame2.data;
	//unsigned char *pixels = frame.data;

	//coord3d = cv::Point3f(0,0,0);

	int count = 0;

	for (int j = 0; j < colorJoint.size(); j++)
	{
		ColorJoint& hd = colorJoint[j];
		hd.coord = cv::Point3f(0, 0, 0);
		hd.points = 0;
	}

	//Update function into current frame
	//bool ok = jointsTracker->update(frame1);
	
	//bool ok;
	//Editar essa parte para o Update Multi Tracker
	for (int i = 0; i < colorJoint.size(); i++)
	{
		ColorJoint& hd = colorJoint[i];

		
		int ok = hd.jointTracker->update(frame1, hd.position);
		
		if (ok)
		{
			//Get the position into the image
			//hd.position = jointsTracker->getObjects()[i];

			//Index the position for determine the space coord
			int w = (hd.position.y + kernnelH) * cColorWidth + (hd.position.x + kernnelH);

			hd.coord.x = m_pCameraCoordinates[w].X;
			hd.coord.y = m_pCameraCoordinates[w].Y;
			hd.coord.z = m_pCameraCoordinates[w].Z;

			//Print tracking on sreen
			rectangle(frame1, hd.position, hd.inputColor, 2, 1);

		}

		else
		{
			putText(frame1, "Tracking failure detected", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

			hd.coord.x = -std::numeric_limits<float>::infinity();
			hd.coord.y = -std::numeric_limits<float>::infinity();
			hd.coord.z = -std::numeric_limits<float>::infinity();
		}
	}


}

int contFrame = 0;

void main() {

	//outFile = fopen("output.txt", "w");
	cv::setBreakOnError(true);
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
	//frame3.create(cColorHeight, cColorWidth, CV_8UC4);
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
					//memcpy(frame3.data, pColorBuffer, nColorBufferSize);
					//cv::cvtColor(frame1, temp, CV_BGRA2BGR);
					//cv::cvtColor(temp, frame2, CV_BGR2HSV);

					updateMarkerPosition();
					//contFrame++;
					//printf("frames: %d\n", contFrame);

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