#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include "Pix.h"

using namespace cv;

const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar SCALAR_BLUE = Scalar(255.0, 0.0, 0.0);
const Scalar SCALAR_GREEN = Scalar(0.0, 255.0, 0.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);

int main(void) {

	VideoCapture capVideo;

	capVideo.open("C:\\Users\\tangd\\Desktop\\Fire Video\\test3.mp4");			//test1 -> test17

	if (!capVideo.isOpened()) {                                                 // neu khong mo duoc file
		std::cout << "\n error reading video file" << std::endl << std::endl;
		return(0);                                                              // va thoat chuong trinh
	}

	char CheckEsc = 0;

	while (capVideo.isOpened() && CheckEsc != 27) {

		Mat Frame1;
		Mat Frame2;

		capVideo.read(Frame1);
		capVideo.read(Frame2);
		imshow("Frame Original", Frame2);
		if (!capVideo.read(Frame1)) {
			break;
		}
		if (!capVideo.read(Frame2))
		{
			break;
		}

		std::vector<Pix> Pixs;

		Mat Frame1Copy = Frame1.clone();
		Mat Frame2Copy = Frame2.clone();

		Mat Difference;
		Mat Thresh;

		cvtColor(Frame1Copy, Frame1Copy, COLOR_BGR2HSV);
		cvtColor(Frame2Copy, Frame2Copy, COLOR_BGR2HSV);

		//imshow("Frame 1 in HSV", Frame1Copy);
		//imshow("Frame 2 in HSV", Frame2Copy);

		inRange(Frame1Copy, Scalar(0, 0, 210), Scalar(70, 190, 255), Frame1Copy);   //Lay nguong cua anh
		inRange(Frame2Copy, Scalar(0, 0, 210), Scalar(70, 190, 255), Frame2Copy);   //Lay nguong cua anh
		//imshow("HSV Range Threshold Fire Frame 1", Frame1Copy);
		//imshow("HSV Range Threshold Fire Frame 2", Frame2Copy);

		absdiff(Frame1Copy, Frame2Copy, Difference);		//so sanh 2 Frame

		threshold(Difference, Thresh, 30, 255.0, CV_THRESH_BINARY);	

		//imshow("Threshold", Thresh);

		Mat structuringElement = getStructuringElement(MORPH_RECT, Size(5, 5));
		dilate(Thresh, Thresh, structuringElement);
		dilate(Thresh, Thresh, structuringElement);
		erode(Thresh, Thresh, structuringElement);

		///////////////Contours
		Mat ThreshCopy = Thresh.clone();
		std::vector<std::vector<Point> > contours;
		findContours(ThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);	//xac dinh vien
		Mat Contours(Thresh.size(), CV_8UC3, SCALAR_BLACK);
		drawContours(Contours, contours, -1, SCALAR_WHITE, -1);
		//imshow("Contours", Contours);

		///////////////ConvexHull
		std::vector<std::vector<Point> > convexHulls(contours.size());
		for (unsigned int i = 0; i < contours.size(); i++) {
			convexHull(contours[i], convexHulls[i]);	//bao loi (bao looif)
		}
		for (auto &convexHull : convexHulls) 
		{
			Pix possiblePix(convexHull);
			if (possiblePix.boundingRect.area() > 50 &&
				possiblePix.dblAspectRatio >= 0.2 &&
				possiblePix.dblAspectRatio <= 1.2 &&
				possiblePix.boundingRect.width > 15 &&
				possiblePix.boundingRect.height > 20 &&
				possiblePix.dblDiagonalSize > 30.0)
				Pixs.push_back(possiblePix);
		}
	
		Mat ConvexHulls(Thresh.size(), CV_8UC3, SCALAR_BLACK);
		convexHulls.clear();
		for (auto &Pix : Pixs) {
			convexHulls.push_back(Pix.contour);
		}
		drawContours(ConvexHulls, convexHulls, -1, SCALAR_WHITE, -1);
		//imshow("ConvexHulls", ConvexHulls);
	
		Frame2Copy = Frame2.clone();          // nhan 1 ban copy nua cua frame 2 vi ta da thay doi bản sao 2 khung trước đó trong quá trình xử lý ở trên

		for (auto &Pix : Pixs) {                                           
			rectangle(Frame2Copy, Pix.boundingRect, SCALAR_BLUE, 2);             // ve khung mau blue xung quanh blod
			circle(Frame2Copy, Pix.centerPosition, 3, SCALAR_GREEN, -1);        // ve mot cham mau green o chinh giua
		}
		imshow("Frame2Copy", Frame2Copy);

		if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) // neu chua het video
		{       
			capVideo.read(Frame2);
		}
		else {
			std::cout << "ket thuc video \n";
			break;                                              // nhay khoi vong lap while
		}
		CheckEsc = waitKey(1);      // neu nhan esc
	}
	if (CheckEsc != 27) {               // neu khong nhan esc
		waitKey(0);                         // giu cua so den loi nhan "end of video"
	}
	return(0);
}