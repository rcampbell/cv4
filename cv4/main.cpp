#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

string intToString(int number){
    
	//this function has a number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}

int main() {
    
    // https://dl.dropboxusercontent.com/u/28096936/tuts/motionTrackingTut/finalCode/motionTracking.cpp
    
    int SENSITIVITY = 60;
    int BLUR_SIZE = 3;
    
    // Contours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Rect objectBoundingRectangle = Rect(0,0,0,0);
    int x = 0, y = 0;
    
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(2,2));
    
    VideoCapture cap(1);
    //cap.set(CV_CAP_PROP_FRAME_WIDTH, 600);
    //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 400);
    
    string windowName = "Death Ray";
    namedWindow(windowName);
    moveWindow(windowName, 0, 0);
    
    Mat previous, current, next;
    Mat raw, result, diff1, diff2;
    cap.read(previous);
    cvtColor(previous, previous, CV_BGR2GRAY);
    cap.read(current);
    cvtColor(current, current, CV_BGR2GRAY);
 
    while(char(waitKey(1)) != 'q') {
        
        // Image processing
        cap.read(next);
        cvtColor(next, next, CV_BGR2GRAY);
        absdiff(previous, next, diff1); // absdiff only works on grayscale images
        absdiff(current, next, diff2);
        bitwise_and(diff1, diff2, result);
        threshold(result, result, SENSITIVITY, 255, CV_THRESH_BINARY);
        blur(result, result, Size(BLUR_SIZE, BLUR_SIZE)); // further reduce noise
        threshold(result, result, SENSITIVITY, 255, CV_THRESH_BINARY);
        erode(result, result, kernel_ero);
        
        // Motion tracking
        //findContours(result, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); // retrieves all contours
   /*     findContours(result, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); // retrieves external contours
        if (contours.size() > 0){
            objectBoundingRectangle = boundingRect(contours.back());
            x = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
            y = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;
            circle(raw, Point(x,y), 20, Scalar(0,255,0), 2);
            line(next, Point(x,y), Point(x,y-25), Scalar(0,255,0), 2);
            line(next, Point(x,y), Point(x,y+25), Scalar(0,255,0), 2);
            line(next, Point(x,y), Point(x-25,y), Scalar(0,255,0), 2);
            line(next, Point(x,y), Point(x+25,y), Scalar(0,255,0), 2);
            putText(next, "Tracking object at (" + intToString(x)+","+intToString(y)+")", Point(x+20,y+20), 1, 1, Scalar(0,255,0), 2);
        } */
   
        imshow(windowName, result);
        previous = current;
        current = next;
    }
    
    cap.release();
    
    return 0;
}


void stereo() {
    VideoCapture capL(1), capR(2);
    
    //reduce frame size
    capL.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    capL.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    capR.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    capR.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    
    namedWindow("Left");
    namedWindow("Right");
    
    while(char(waitKey(1)) != 'q') {
        
        //grab raw frames first
        capL.grab();
        capR.grab();
        
        //decode later so the grabbed frames are less apart in time
        Mat frameL, frameR;
        capL.retrieve(frameL);
        capR.retrieve(frameR);
        
        if(frameL.empty() || frameR.empty()) break;
        
        imshow("Left", frameL);
        imshow("Right", frameR);
    }
    
    capL.release();
    capR.release();
}

void backgroundSubtraction() {
    
    // Tracking moving objects via background subtraction, see:
    // http://mateuszstankiewicz.eu/?p=189
    
    VideoCapture cap(1);
    
    //BackgroundSubtractorMOG subtractor(3, 5, 0.7, 0);
    BackgroundSubtractorMOG2 subtractor(3, 16, false);
    
    vector<vector<Point> > contours;
    
    namedWindow("Camera");
    //namedWindow("Background");
    
    while(char(waitKey(1)) != 'q') {
        cap.grab();
        Mat frame, background, foreground;
        cap.retrieve(frame);
        subtractor.operator ()(frame, foreground);
        subtractor.getBackgroundImage(background);
        erode(foreground, foreground, Mat());
        dilate(foreground, foreground, Mat());
        findContours(foreground, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        drawContours(frame, contours, -1, Scalar(0,0,255), 2);
        if (contours.size() > 0) {
            vector<Point> largest = contours.back(); // the largest contour is at the end
            Rect bound = boundingRect(largest);
            rectangle(frame, bound.tl(), bound.br(), Scalar(0,255,255));
        }
        
        
        imshow("Camera", frame);
        //imshow("Background", background);
    }
    
    cap.release();
}