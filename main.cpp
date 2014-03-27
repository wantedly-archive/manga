// main.cpp
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>

using namespace std;
using namespace cv;

string basename(const string& path) {
  return path.substr(path.find_last_of('/') + 1);
}

int main(int argc, char* argv[]) {
  // load color image
  const char* imagename = argc > 1 ? argv[1] : "./img/lena.png";
  IplImage* cvsrc = cvLoadImage(imagename, 1);

  if (cvsrc == NULL) {
    cout << "file not found" << endl;
    return -1;
  }
  
  // pyramid segmentation
  CvMemStorage *storage = 0;
  CvSeq *comp = 0;
  storage = cvCreateMemStorage (0);
  IplImage *cvpyr = cvCloneImage (cvsrc);
  cvPyrSegmentation (cvsrc, cvpyr, storage, &comp, 4, 255.0, 50.0);
  cvReleaseMemStorage (&storage);
  Mat pyr_img(cvpyr);

  // convert color image to grayscale
  Mat src_img(cvsrc);
  Mat gray_img;
  cvtColor(src_img, gray_img, CV_BGR2GRAY);

  Mat bin_img;
  adaptiveThreshold(gray_img, bin_img, 255,
    CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 7, 11);

  Mat dst_img;
  bitwise_and(pyr_img, bin_img, dst_img);

  string new_imagename = "./new_";
  new_imagename += basename(string(imagename));

  if (imwrite(new_imagename, bin_img)) {
    cout << "imwrite:" << new_imagename << " ... success" << endl;
  } else {
    cout << "imwrite:" << new_imagename << " ... failure" << endl;
  }

  return 0;
}
