#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"

#include <stdio.h>
#include <sstream>

using namespace cv;

void view_init();
bool view_loop(Mat m, Mat c, float factor);
void view_term();



bool pause = false;

void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		pause = !pause;
	}
	if (event == EVENT_LBUTTONUP)
	{
	}
}

int main(int argc, char** argv)
{

	enum { STEREO_BM = 0, STEREO_SGBM = 1, STEREO_HH = 2, STEREO_VAR = 3, STEREO_3WAY = 4, STEREO_HH4 = 5 };
	int alg = STEREO_BM;
	int SADWindowSizeIndex = 0;
	int SADWindowSize = 5 + SADWindowSizeIndex * 2;
	int numberOfDisparitiesIndex = 0;
	int numberOfDisparities = (numberOfDisparitiesIndex+1)*16;
	int targetwith = 200;
	int frame = 0;
	int div = 4;
	bool no_display = false;
	bool color_display = true;


	//VideoCapture cap("test.mp4");
	VideoCapture cap("San Francisco in Stereo 3D - SBS.mp4");
	
	if (!cap.isOpened()) {
		return -1;
	}


	view_init();

	int nframe = (int)cap.get(CAP_PROP_FRAME_COUNT);
	cap.set(CAP_PROP_POS_FRAMES, 850);
	

	Ptr<StereoBM> bm = StereoBM::create(16, 9);
	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16, 3);


	namedWindow("SBS23D");
	createTrackbar("frames", "SBS23D", &frame, nframe, NULL);
	createTrackbar("disp index", "SBS23D", &numberOfDisparitiesIndex, 4, NULL);
	createTrackbar("block", "SBS23D", &SADWindowSizeIndex, 255, NULL);
	createTrackbar("width", "SBS23D", &targetwith, 640, NULL);
	createTrackbar("div", "SBS23D", &div, 20, NULL);
	setTrackbarPos("div", "SBS23D", 4);

	setMouseCallback("SBS23D", callBackFunc);

			
	int color_mode = alg == STEREO_BM ? 0 : -1;
	Mat imgcolor;



	while (true) {

		numberOfDisparities = (numberOfDisparitiesIndex + 1) * 16;
		SADWindowSize = 5 + SADWindowSizeIndex * 2;

		if (!pause) {
			int f = getTrackbarPos("frames", "SBS23D");
			setTrackbarPos("frames", "SBS23D", f + 1);
		}

		cap.set(CAP_PROP_POS_FRAMES, frame);

		
		cap >> imgcolor;
		
		if (imgcolor.cols == 0) {
			break;
		}
		
		Mat img;
		cvtColor(imgcolor, img, COLOR_RGB2GRAY);
		Rect r1(0, 0, img.cols / 2, img.rows);
		Rect r2(img.cols / 2, 0, img.cols / 2, img.rows);
		Mat img1 = img(r2);
		Mat img2 = img(r1);
		Mat img1color = imgcolor(r2);


		float scale = (float)(targetwith>50? targetwith:50) / img1.cols;
		if (scale > 1.0f) scale = 1.0f;


		if (scale != 1.f)
		{
			Mat temp1, temp2;
			int method = scale < 1 ? INTER_AREA : INTER_CUBIC;
			resize(img1, temp1, Size(), scale, scale, method);
			img1 = temp1;
			resize(img2, temp2, Size(), scale, scale, method);
			img2 = temp2;
		}

		Size img_size = img1.size();

		int minsize = std::min(img1.cols, img1.rows);
		if (SADWindowSize >= minsize) {
			SADWindowSize = (minsize-2) + (minsize-2) % 2;
		}

		Rect roi1, roi2;
		

		numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width / 8) + 15) & -16;

		bm->setROI1(roi1);
		bm->setROI2(roi2);
		bm->setPreFilterCap(31);
		bm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 9);
		bm->setMinDisparity(0);
		bm->setNumDisparities(numberOfDisparities);
		bm->setTextureThreshold(10);
		bm->setUniquenessRatio(15);
		bm->setSpeckleWindowSize(100);
		bm->setSpeckleRange(32);
		bm->setDisp12MaxDiff(1);

		sgbm->setPreFilterCap(63);
		int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
		sgbm->setBlockSize(sgbmWinSize);

		int cn = img1.channels();

		sgbm->setP1(8 * cn * sgbmWinSize * sgbmWinSize);
		sgbm->setP2(32 * cn * sgbmWinSize * sgbmWinSize);
		sgbm->setMinDisparity(0);
		sgbm->setNumDisparities(numberOfDisparities);
		sgbm->setUniquenessRatio(10);
		sgbm->setSpeckleWindowSize(100);
		sgbm->setSpeckleRange(32);
		sgbm->setDisp12MaxDiff(1);
		if (alg == STEREO_HH)
			sgbm->setMode(StereoSGBM::MODE_HH);
		else if (alg == STEREO_SGBM)
			sgbm->setMode(StereoSGBM::MODE_SGBM);
		else if (alg == STEREO_HH4)
			sgbm->setMode(StereoSGBM::MODE_HH4);
		else if (alg == STEREO_3WAY)
			sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);

		Mat disp, disp8;


		int64 t = getTickCount();
		float disparity_multiplier = 1.0f;
		if (alg == STEREO_BM)
		{
			bm->compute(img1, img2, disp);
			if (disp.type() == CV_16S)
				disparity_multiplier = 16.0f;
		}
		else if (alg == STEREO_SGBM || alg == STEREO_HH || alg == STEREO_HH4 || alg == STEREO_3WAY)
		{
			sgbm->compute(img1, img2, disp);
			if (disp.type() == CV_16S)
				disparity_multiplier = 16.0f;
		}
		t = getTickCount() - t;

		if (alg != STEREO_VAR)
			disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities * 16.));
		else
			disp.convertTo(disp8, CV_8U);

		Mat disp8_3c;
		if (color_display)
			cv::applyColorMap(disp8, disp8_3c, COLORMAP_DEEPGREEN);

		if (!no_display)
		{
			std::ostringstream oss;
			oss << "disparity  " << (alg == STEREO_BM ? "bm" :
				alg == STEREO_SGBM ? "sgbm" :
				alg == STEREO_HH ? "hh" :
				alg == STEREO_VAR ? "var" :
				alg == STEREO_HH4 ? "hh4" :
				alg == STEREO_3WAY ? "sgbm3way" : "");
			oss << "  blocksize:" << (alg == STEREO_BM ? SADWindowSize : sgbmWinSize);
			oss << "  max-disparity:" << numberOfDisparities;
			oss << "  Time elapsed:" << (t * 1000 / getTickFrequency());
			std::string disp_name = oss.str();


			if (getWindowProperty("SBS23D", WND_PROP_AUTOSIZE) == -1) {
				break;
			}

			Mat overlay = color_display ? disp8_3c : disp8;
			Mat insetImage(imgcolor, Rect(0, 0, overlay.cols, overlay.rows));
			overlay.copyTo(insetImage);
			putText(imgcolor, disp_name, Point2f(5, (float)(imgcolor.rows-5)), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255));
			imshow("SBS23D", imgcolor);

			Mat floatDisp;
			Mat xyz;
			disp.convertTo(floatDisp, CV_32F, 1.0f / disparity_multiplier);

			Mat color;
			resize(img1color, color, Size(floatDisp.cols, floatDisp.rows));

			if (waitKey(1) == 27) //ESC (prevents closing on actions like taking screenshots)
			{
				break;
			}
			try {
				if (!view_loop(floatDisp, color, 1.0 / div)) {
					break;
				}
			} 
			catch (...)
			{

			}
		}
	}

	view_term();

	return 0;
}

