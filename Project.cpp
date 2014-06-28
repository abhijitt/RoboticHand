#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <math.h>
#include "serial.h"

using namespace std;
using namespace cv;

int thresh[4];
int color[5];
int avgcolor[3][3];
bool curled[4];
class My_ROI{
public:
My_ROI();
My_ROI(Point upper_corner, Point lower_corner,Mat src);
Point upper_corner, lower_corner;
Mat roi_ptr;
Scalar color;
int border_thickness;
void draw_rectangle(Mat src);
};

My_ROI::My_ROI(){
upper_corner=Point(0,0);
lower_corner=Point(0,0);

}

My_ROI::My_ROI(Point u_corner, Point l_corner, Mat src){
upper_corner=u_corner;
lower_corner=l_corner;
color=Scalar(0,255,0);
border_thickness=1;
roi_ptr=src(Rect(u_corner.x, u_corner.y, l_corner.x-u_corner.x,l_corner.y-u_corner.y));
}

void My_ROI::draw_rectangle(Mat src){
rectangle(src,upper_corner,lower_corner,color,1,8,0);
}
int square_len=20;    
vector <My_ROI> roi;
VideoCapture cap;

void CallBackFunc(int event, int x, int y, int flags, Mat src)
{
if  ( event == EVENT_LBUTTONDOWN )
{
cout<<src.at<Vec3b>(y,x)[0]<<" ";
cout<<src.at<Vec3b>(y,x)[1]<<" ";
cout<<src.at<Vec3b>(y,x)[2]<<" "<<endl;
}
}
int minimum;
int getMedian(vector<int> val);
void getAvgColor(Mat src,My_ROI roi,int avg[3]);
void waitForPalmCover(Mat, VideoCapture);
void average(Mat src,int avgcolor[6][3]);
Mat produceBinary(Mat src, bool curled[4],int minimum);
void  initTrackbar();
Mat src;
void detectHand(Mat I,vector<vector <Point> >& contours, vector<Vec4i> & hierarchy,Point a[]);
void findBiggestContour(vector<vector<Point> > contours,int index[2]);
int main()
{

int distance,distance1;
Point center[2];
Mat c;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
serial_device arduino;	
arduino.initialize("/dev/ttyACM0");
bool a[6];
int result=0;
VideoCapture cap(0);
waitForPalmCover(src, cap);
cap.read(src);
average(src,avgcolor);
for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
			{
			cout<<avgcolor[i][j]<<" ";		
			}
	cout<<endl;
	}
namedWindow("Trackbar",WINDOW_AUTOSIZE);
initTrackbar();
minimum=50000;
while(1)
	{
	cap.read(src);
	result=0;	
	cvtColor(src,src,CV_RGB2HSV);
	c=produceBinary(src,curled,minimum); 
	for(int i=0; i<6; i++)
		{
			result+=pow(2,i)*(int)curled[i];
		}
	cout<<result<<endl;	
	arduino.write_byte(char(result));
	//imshow("BINARY",bw);	
	imshow("Video",src);	
	if(waitKey(30)>=0) break;
	}
}


void waitForPalmCover(Mat src,VideoCapture cap){
if(cap.read(src)){
flip(src,src,1);
roi.push_back(My_ROI(Point(src.cols/3, src.rows/3),Point(src.cols/3+square_len,src.rows/3+square_len),src));
roi.push_back(My_ROI(Point(src.cols/3, src.rows/3+square_len),Point(src.cols/3+square_len,src.rows/3+2*square_len),src));

//roi.push_back(My_ROI(Point(src.cols/3, src.rows/3+2*square_len),Point(src.cols/3+square_len,src.rows/6+3*square_len),src));
}


for(int i =0;i<70;i++)
	{
	if(cap.read(src)){
	flip(src,src,1);
	for(int j=0;j<2;j++)
		{
		roi[j].draw_rectangle(src);
		}
	
	imshow("Video", src);        
	if(cv::waitKey(30) >= 0) 
		break;
	}
	}
}

int getMedian(vector<int> val)
{
int median;
sort(val.begin(),val.end());
if(val.size() % 2 ==0) median=val[val.size()/2 -1];
else median=val[val.size()/2];
return median;
}

void getAvgColor(Mat src,vector<My_ROI> roi,int avg[6][3])
{
	for(int k=0; k<2; k++)
	{
	Mat r;
	uchar* p;
	vector<int> hm,sm,lm;
	r=roi[k].roi_ptr;
	for(int i=0; i<r.rows; ++i)
		{
		for(int j=0; j<r.cols; ++j)
			{
			hm.push_back(r.at<Vec3b>(i,j)[0]);
			sm.push_back(r.at<Vec3b>(i,j)[1]);		
			lm.push_back(r.at<Vec3b>(i,j)[2]);		
			}
		}
	avg[k][0]=getMedian(hm);
	avg[k][1]=getMedian(sm);
	avg[k][2]=getMedian(lm);	
	}

}
void average(Mat src,int avgcolor[3][3])
{
	flip(src,src,1);	
	cvtColor(src,src,CV_RGB2HSV);
	getAvgColor(src,roi,avgcolor);
	imshow("Video",src);

}

Scalar getLowerLimit(int avgcolor[3],int lower[3])
{
Scalar a;
a=(avgcolor[0]-lower[0],avgcolor[0]-lower[1],avgcolor[0]-lower[2]);
return a;
}

Scalar getUpperLimit(int avgcolor[3],int upper[3])
{
Scalar a;
a=(avgcolor[0]+upper[0],avgcolor[0]+upper[1],avgcolor[0]+upper[2]);
return a;
}


Mat produceBinary(Mat src,bool curled[6],int minimum)
{	
	Moments m;	
	Mat bw[5];
	Mat b;	
	/*for(int i=0; i<2; i++)
	{
	inRange(src,getLowerLimit(avgcolor[i],lower),getUpperLimit(avgcolor[i],upper),b);
	if(i==0) {bw[0]=b; imshow("bw",b);}	
	bitwise_or(b,bw[0],bw[0]);
	}*/
	inRange(src,Scalar(color[0]-10,100,100),Scalar(color[0]+10,255,255),bw[0]); //Blue
	inRange(src,Scalar(color[1]-10,100,100),Scalar(color[1]+10,255,255),bw[1]); //Red
	inRange(src,Scalar(color[2]-10,100,100),Scalar(color[2]+10,255,255),bw[2]); //Green  
	inRange(src,Scalar(color[3]-10,100,100),Scalar(color[3]+10,255,255),bw[3]); //Yellow
	inRange(src,Scalar(color[4]-10,100,100),Scalar(color[4]+10,255,255),bw[4]);	
	/*Scalar values for Green: Scalar(0,140,60),Scalar(20,160,120)
	Scalar values for Red: Scalar(120,10,80),Scalar(140,180,120)
	Scalar values for Blue: Scalar(5,180,80),Scalar(15,220,120)
	*/
	
	for(int i=0; i<4; i++)
	{
	erode(bw[i],bw[i],0,Point(-1,-1),2);
	
	m=moments(bw[i],false);
	if(m.m00>thresh[i]) curled[i]=true;
	else curled[i]=false;
	cout<<curled[i]<<" ";	
	}
	erode(bw[4],bw[4],0,Point(-1,-1),2);
	m=moments(bw[4],false);
		
	if(m.m00>minimum && m.m00<(3*minimum)) { curled[4]=false; curled[5]=true;}
	else if(m.m00>(minimum*3) && m.m00 <(minimum*4)) { curled[4]=true; curled[5]=false;}
	else if(m.m00>(minimum*4)) { curled[4]=true; curled[5]=true;}
	else { curled[4]=false; curled[5]=false;}
	cout<<curled[4]<<" "<<curled[5]<<endl;
	cout<<" "<<m.m00<<" ";
	imshow("BW",bw[0]);
	imshow("BW1",bw[1]);
	imshow("BW2",bw[2]);
	imshow("BW3",bw[3]);
return bw[1];
}






void  initTrackbar()
{
//lower[0]=10; lower[1]=10; lower[2]=10;
//upper[0]=10; upper[1]=10; upper[2]=10;
createTrackbar("Blue","Trackbar",&color[0],179);
createTrackbar("Red","Trackbar",&color[1],179);
createTrackbar("Green","Trackbar",&color[2],179);
createTrackbar("Yellow","Trackbar",&color[3],179);
createTrackbar("Pink","Trackbar",&color[4],179);
createTrackbar("threshold-Blue","Trackbar",&thresh[0],100000);
createTrackbar("threshold-Red","Trackbar",&thresh[1],100000);
createTrackbar("threshold-Green","Trackbar",&thresh[2],100000);
createTrackbar("threshold-Yellow","Trackbar",&thresh[3],100000);
}


/*Scalar values for Green: Scalar(0,140,60),Scalar(20,160,120)
Scalar values for Red: Scalar(120,120,80),Scalar(140,180,120)
Scalar values for Blue: Scalar(5,180,80),Scalar(15,220,120)
*/
//Use the following command for interfacing
//stty -F /dev/tty.usbmodem641 sane raw pass8 -echo -hupcl clocal 9600
