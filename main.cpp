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

  Mat src_img = imread(imagename);
  if (!src_img.data) {
    cout << "file not found" << endl;
    return -1;
  }

  cout << src_img.size() << endl;

  Mat resize_img(512, 2048, CV_64FC(3));
  resize(src_img, resize_img, resize_img.size());

  cout << resize_img.size() << endl;

  // convert color image to grayscale
  Mat gray_img;
  cvtColor(resize_img, gray_img, CV_BGR2GRAY);

  Mat bin_img;
  adaptiveThreshold(gray_img, bin_img, 255,
    CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 5, 10);
  Mat rgb_bin_img;
  cvtColor(bin_img, rgb_bin_img, CV_GRAY2BGR);

  // cvPyrSegmentationを使うため、IplImageにキャスト
  IplImage cvresize = resize_img;
  IplImage *cvresize_p = &cvresize;
  
  // pyramid segmentation
  CvMemStorage *storage = 0;
  CvSeq *comp = 0;
  storage = cvCreateMemStorage (0);
  IplImage *cvpyr = cvCloneImage (cvresize_p);
  cvPyrSegmentation (cvresize_p, cvpyr, storage, &comp, 4, 255.0, 50.0);
  cvReleaseMemStorage (&storage);
  Mat pyr_img(cvpyr);

  Mat and_img;
  bitwise_and(pyr_img, rgb_bin_img, and_img);
  
  string new_imagename = "./new_img/";
  new_imagename += basename(string(imagename));

  Mat dst_img;
  resize(and_img, dst_img, src_img.size());

  if (imwrite(new_imagename, dst_img)) {
    cout << "imwrite:" << new_imagename << " ... success" << endl;
  } else {
    cout << "imwrite:" << new_imagename << " ... failure" << endl;
  }

  return 0;
}
