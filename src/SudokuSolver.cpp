// SudokuSolver.cpp : Solves Sudoku Puzzle using Computer Vision

#include "stdafx.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include<tesseract/baseapi.h>
#include<iostream>
#include "solve.h"


using namespace cv;
using namespace std;
using namespace tesseract;

//#define KERNEL_RATIO 0.004
//#define MAX_WIDTH 1200
//#define THRESHOLD_C 7	//ALSO DEPENDENT ON MAX_WIDTH???
//#define HOUGH_r_RES 1
//#define BOX_SIZE 40
//#define PT_CLOSE 20 //SHOULD DEPEND ON MAX_WIDTH?

int PT_CLOSE;

bool isParallel(const Vec2f& a, const	Vec2f& b)
{
	float theta1 = a[1];
	float theta2 = b[1];

	if (abs(theta1 - theta2) < 20.0 * CV_PI / 180 || abs(theta1 - theta2) > 160.0 * CV_PI / 180)
		return true;
	else
		return false;
}

bool isClose(const Point a, const Point b)
{
	double dist = sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));

	if (dist < PT_CLOSE)
		return true;
	else
		return false;
}

bool row_major(const Point& a, const Point& b)
{
	if (abs(a.y - b.y) < 25)
	{
		if (a.x < b.x)
			return true;
		else
			return false;
	}
	else
	{
		if (a.y < b.y)
			return true;
		else
			return false;
	}
}

bool liClose(const Vec2f line1, const Vec2f line2)
{
	double rho1 = line1[0], theta1 = line1[1];
	double rho2 = line2[0], theta2 = line2[1];

	double rho;
	double factor = (cos(theta1)*cos(theta2) + sin(theta1)*sin(theta2));

	if (factor == 0)
		return false;
	else
	{
		rho = rho2 / factor;

		if (abs(rho - rho1) < 15)
			return true;
		else
			return false;
	}
}

bool byDistance(const Vec2f& line1, const Vec2f& line2)
{
	if (line1[0] < line2[0])
		return true;
	else
		return false;
}


class SudokuSolver
{
	Mat image;
	int width;
	int height;
	double diagonal;
	const double KERNEL_RATIO = 0.004;
	short kernel_size;
	const int MAX_WIDTH = 1200;
	int THRESHOLD_C;
	const int HOUGH_r_RES = 1;
	int PT_CLOSE;
	int HOUGH_THRESHOLD;
	string frame_count;



	Point polar_solver(const Vec2f& line1, const Vec2f& line2)
	{
		double rho1 = line1[0], theta1 = line1[1];
		double rho2 = line2[0], theta2 = line2[1];

		Mat LHS = Mat(2, 2, CV_64F);
		LHS.at<double>(0, 0) = cos(theta1);
		LHS.at<double>(0, 1) = sin(theta1);
		LHS.at<double>(1, 0) = cos(theta2);
		LHS.at<double>(1, 1) = sin(theta2);

		Mat RHS = Mat(2, 1, CV_64F);
		RHS.at<double>(0, 0) = rho1;
		RHS.at<double>(1, 0) = rho2;

		Mat ans = Mat(2, 1, CV_64F);

		solve(LHS, RHS, ans);
		Point p;
		p.x = ans.at<double>(0, 0);
		p.y = ans.at<double>(1, 0);

		return p;
	}

	bool getTenLines(vector<Vec2f>& v)
	{
		sort(v.begin(), v.end(), byDistance);
		int width = 80;
		vector<Vec2f> ans;
		int count = 0;
		auto it = v.begin();
		while (count != 10 && it != v.end())
		{
			if (ans.empty())
			{
				ans.push_back(*(it++));
				count++;
			}
			else
			{
				int dist = abs(abs((*it)[0]) - abs(ans.back()[0]));
				if (dist < 150)
				{
					if (dist < 40)
					{
						ans.push_back(*(it++));
					}
					else
					{
						ans.push_back(*(it++));
						count++;
					}
				}

				else
				{
					count = 0;
					ans.clear();
				}
			}
		}

		if (count == 10)
		{
			v = ans;
			return true;
		}
		else
		{
			v = ans;
			return false;
		}
	}
public:
	int BOX_SIZE;

	SudokuSolver(Mat& image)
	{
		this->image = image;
		width = image.size().width;
		height = image.size().height;
		cout << "Image Width: " << width<<endl;
		cout << "Image Height: " << height<<endl;

		frame_count = "0";

		//Resize to appropriate dimensions
		if (width > MAX_WIDTH)
		{
			unsigned int new_width = MAX_WIDTH;
			unsigned int new_height = ((double)MAX_WIDTH) / width * height;
			resize(image, image, Size(new_width, new_height));

			width = new_width;
			height = new_height;
		}

		//Find Diagonal
		diagonal = sqrt(pow(width, 2) + pow(height, 2));
		cout << "Resized Image Diagonal: " << diagonal << endl;

		//Find appropriate kernel size: 0.4% of image diagonal
		kernel_size = KERNEL_RATIO * width;
		kernel_size = (kernel_size % 2 == 0 ? kernel_size + 1 : kernel_size);

		if (kernel_size < 3)
			kernel_size = 3;
		cout << "Using Kernel Size: " << kernel_size << endl;
		
		PT_CLOSE = (diagonal / 75 > 30 ? diagonal / 75 : 30);
		::PT_CLOSE = PT_CLOSE;
		THRESHOLD_C = 5;
		BOX_SIZE = 40;
		HOUGH_THRESHOLD = (width / 3 > 300 ? width / 3 : 300);
		//Print resized image
		image = this->image;
		print();
	}

	void print(Mat& img = Mat(), string s = "")
	{
		if (img.empty())
		{
			namedWindow("Original", WINDOW_NORMAL);
			imshow("Original", image);
		}

		else
		{
			if (s.empty())
			{
				namedWindow(frame_count, WINDOW_NORMAL);
				imshow(frame_count, img);
				waitKey(0);
				//destroyWindow(frame_count);
				frame_count[0] = (int)frame_count[0]++;
			}
			else
			{
				namedWindow(s, WINDOW_NORMAL);
				imshow(s, img);
				waitKey(0);
				//destroyWindow(s);
			}
		}
	}

	void preProcess(Mat& filter_img)
	{

		//Convert to Grayscale
		Mat gray_img;
		cvtColor(image, gray_img, COLOR_BGR2GRAY);

		//Mat blur_img;
		//blur(gray_img, blur_img, Size(kernel_size, kernel_size));
		//namedWindow("Blurred", WINDOW_NORMAL);
		//imshow("Blurred", blur_img);


		//(Inverted) Binarisation of grayscale image
		Mat thresh_img;
		adaptiveThreshold(gray_img, thresh_img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, kernel_size, THRESHOLD_C);
		print(thresh_img, "Threshold");


		//Smoothening of binary image
		bilateralFilter(thresh_img, filter_img, kernel_size, 100, 50);

		//Closing of holes
		//Mat kernelMorph = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));
		//morphologyEx(filter_img, filter_img, MORPH_OPEN, kernelMorph);
		print(filter_img, "Filtered");

		//Mat canny_img;
		//Canny(filter_img, canny_img, 30, 90);
		//namedWindow("Canny", WINDOW_NORMAL);
		//imshow("Canny", canny_img);
		//filter_img = canny_img;
	}

	void drawLines(Mat& image, vector<Vec2f>& lines, Scalar& color)
	{
		for (size_t i = 0; i < lines.size(); i++)
		{
			float rho = lines[i][0], theta = lines[i][1];
			Point pt1, pt2, pt3, pt4;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = cvRound(x0 + diagonal * (-b));
			pt1.y = cvRound(y0 + diagonal * (a));
			pt2.x = cvRound(x0 - diagonal * (-b));
			pt2.y = cvRound(y0 - diagonal * (a));

			line(image, pt1, pt2, color, 2);
		}
	}

	bool houghTransform(Mat& filter_img, vector<Vec2f>& vLines, vector<Vec2f>& hLines)
	{
		vector<Vec2f> lines;
		HoughLines(filter_img, lines, HOUGH_r_RES, CV_PI / 180, HOUGH_THRESHOLD);

		//Partition into Vertical/Horizontal
		vector<int> labels;
		partition(lines, labels, isParallel);

		for (auto i : labels)
			cout << i << ' ';
		cout << endl;

		vector<unsigned int> vLabels, hLabels;
		for (int i = 0; i < lines.size(); i++)
		{
			if (labels[i] == 0)
				vLines.push_back(lines[i]);
			else if (labels[i] == 1)
				hLines.push_back(lines[i]);
		}




		if (!getTenLines(vLines) || !getTenLines(hLines))
		{
			cout << "Couldn't get 10 blocks of lines :\\\\\\" << endl;
			return false;
		}

		return true;
	}

	bool intersections(vector< vector<Point> >& Grid, vector<Vec2f>& vLines, vector<Vec2f>& hLines)
	{
		vector<Point> intersection_points;
		for (int i = 0; i<vLines.size(); i++)
		{
			for (int j = 0; j<hLines.size(); j++)
			{
				intersection_points.push_back(polar_solver(vLines[i], hLines[j]));
			}
		}

		//Coglomerate multiple close points
		vector<int> pLabels;
		int num_of_points = partition(intersection_points, pLabels, isClose);

		vector<Point> grid_points(num_of_points);
		vector<int> freq(num_of_points, 0);

		cout << "Number of Points: " << num_of_points;

		if (num_of_points != 100)
		{
			cout << "Unable to find all boxes :(" << endl;
			return false;
		}
		//assert(num_of_points == 100);
		int currLabel;
		for (int p = 0; p < intersection_points.size(); p++)
		{
			currLabel = pLabels[p];
			grid_points[currLabel].x += intersection_points[p].x;
			grid_points[currLabel].y += intersection_points[p].y;
			freq[currLabel]++;
		}

		for (int p = 0; p < grid_points.size(); p++)
		{
			grid_points[p].x /= freq[p];
			grid_points[p].y /= freq[p];
		}

		//TO DO: GET THE GRID POINTS FROM ALL THE POINTS
		//RIGHT NOW, ASSUMING THAT GIVEN IMAGE HAS NOTHING OTHER THAN THE PUZZLE

		sort(grid_points.begin(), grid_points.end(), row_major);

		Grid.resize(10);
		auto it = grid_points.begin();
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				Grid[i].push_back(*(it++));
			}
		}
	}

	void boxes(Rect rectCords[9][9], Mat rectGrid[9][9], vector< vector<Point> > Grid, int offset = 15)
	{
		Mat filterTemp;
		Mat kernelDilateNum = getStructuringElement(MORPH_RECT, Size(3, 3));

		cout << "\n Before Processing!";
		//Processing Grid Boxes
		for (int i = 0; i<Grid.size() - 1; i++)
		{
			for (int j = 0; j < Grid[i].size() - 1; j++)
			{
				int x = Grid[i][j].x + offset;
				int y = Grid[i][j].y + offset;
				int x_ = Grid[i + 1][j + 1].x - offset;
				int y_ = Grid[i + 1][j + 1].y - offset;

				rectCords[i][j] = Rect(Point(x, y), Point(x_, y_));
				image(rectCords[i][j]).copyTo(rectGrid[i][j]);
				cvtColor(rectGrid[i][j], rectGrid[i][j], COLOR_BGR2GRAY);
				adaptiveThreshold(rectGrid[i][j], rectGrid[i][j], 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 5);
				bilateralFilter(rectGrid[i][j], filterTemp, 3, 100, 50);
				morphologyEx(filterTemp, rectGrid[i][j], MORPH_CLOSE, kernelDilateNum);
				//rectangle(image, Point(x, y), Point(x_, y_), Scalar(0, 165, 255), 2);
			}
		}

	}

	void drawRectangles(Mat& drawImg, Rect rectCords[9][9], Scalar color = Scalar(0, 165, 255))
	{
		for (int i = 0; i<9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				rectangle(drawImg, rectCords[i][j], color, 2);
			}
		}
	}

};

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "Please provide with image location!";
		return -1;
	}

	//Read the image if it exists
	Mat image;
	image = imread(argv[1], IMREAD_COLOR);
	if (image.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}
	
	SudokuSolver solver(image);
	//Preprocess
	Mat filter_img;
	solver.preProcess(filter_img);

	vector<Vec2f> vLines, hLines;
	solver.houghTransform(filter_img, vLines, hLines);
	
	Mat image_lines;
	image.copyTo(image_lines);
	solver.drawLines(image_lines, vLines, Scalar(255, 0, 0));
	solver.drawLines(image_lines, hLines, Scalar(0, 255, 0));
	solver.print(image_lines, "Hough Transform");

	vector< vector<Point> > Grid;
	solver.intersections(Grid, vLines, hLines);
	

	Mat rectGrid[9][9];
	Rect rectCords[9][9];
	solver.boxes(rectCords, rectGrid, Grid);
	Mat boxImg;
	image.copyTo(boxImg);
	solver.drawRectangles(boxImg, rectCords);
	solver.print(boxImg, "With Boxes");

	//Tesseract Initialise
	TessBaseAPI *api = new TessBaseAPI();
	api->Init(".\\", "eng");
	api->SetVariable("debug_file", "just_delete_:).log");
	api->SetPageSegMode(PageSegMode(PSM_SINGLE_CHAR));
	api->SetVariable("tessedit_char_blacklist", "!?@#$%&*()<>_-+=/:;'\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	api->SetVariable("tessedit_char_whitelist", "0123456789");
	api->SetVariable("classify_bln_numeric_mode", "1");


	short sudoku[9][9];
	char* solve_string = new char[82];
	Mat temp_block;
	char* c;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			temp_block = rectGrid[i][j];
			api->SetImage((uchar*)temp_block.data, temp_block.size().width, temp_block.size().height, temp_block.channels(), temp_block.step1());
			c = api->GetUTF8Text();
			if (isdigit(c[0]))
			{
				sudoku[i][j] = int(c[0]) - 48;
				solve_string[i * 9 + j] = c[0];
			}
			else
			{
				sudoku[i][j] = 0;
				solve_string[i * 9 + j] = '0';
			}
		}
	}

	for (int i = 0; i < 9; i++)
	{
		cout << endl;
		for (int j = 0; j < 9; j++)
			cout << sudoku[i][j] << ' ';
	}

	//Solving String
	solve_sudoku* sol = new solve_sudoku();
	solve_string[81] = '\0';
	cout << "Unsolved String" << solve_string;
	sol->solve_puzzle(solve_string);
	solve_string[81] = '\0';
	cout << '\n' << solve_string;


	//Put Digits in the Image
	//Mat rectang;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (sudoku[i][j] == 0)
			{
				string s;
				s.push_back(solve_string[i * 9 + j]);
				Point a = rectCords[i][j].tl();
				a.y += 40;
				a.x += 5;
				putText(image, s, a, 1, 3, Scalar(0, 0, 255), 3);

				//image(rectCords[i][j]) = rectang;
			}
		}
	}

	//Show Final Image
	namedWindow("Orig", WINDOW_NORMAL);
	imshow("Orig", image); 

	delete api;
	delete sol;
	waitKey(0);
	return 0;
}