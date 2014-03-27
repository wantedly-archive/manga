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

unsigned int AlignPow2(unsigned int a) {
  unsigned int i = 1;
  while (a > (i <<= 1)) {
    if (!i) { break; }
  }
  return i;
}

int main(int argc, char* argv[]) {
  // load color image
  const char* imagename = argc > 1 ? argv[1] : "./img/lena.png";

  Mat src_img = imread(imagename);
  if (!src_img.data) {
    cout << "file not found" << endl;
    return -1;
  }

  Size size = src_img.size();
  Mat resize_img(AlignPow2(size.height), AlignPow2(size.width), CV_64FC(3));
  resize(src_img, resize_img, resize_img.size());

  cout << "src_img: " << src_img.size() << endl;
  cout << "resize_img: " << resize_img.size() << endl;

  // convert color image to grayscale
  Mat gray_img;
  cvtColor(resize_img, gray_img, CV_BGR2GRAY);

  Mat bin_img;
  adaptiveThreshold(gray_img, bin_img, 255,
    CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 10);

  // cvPyrSegmentationを使うため、IplImageにキャスト
  IplImage cvresize = resize_img;
  IplImage *cvresize_p = &cvresize;
  
  // pyramid segmentation
  CvMemStorage *storage = 0;
  CvSeq *comp = 0;
  storage = cvCreateMemStorage (0);
  IplImage *cvpyr = cvCloneImage (cvresize_p);
  cvPyrSegmentation (cvresize_p, cvpyr, storage, &comp, 8, 255.0, 50.0);
  cvReleaseMemStorage (&storage);
  Mat pyr_img(cvpyr);

  Mat gray_pyr_img;
  cvtColor(pyr_img, gray_pyr_img, CV_BGR2GRAY);
  
  // 画像を重ねる
  Mat and_img;
  bitwise_and(bin_img, gray_pyr_img, and_img);
  
  // src_imgのサイズに戻す
  Mat dst_img;
  resize(and_img, dst_img, src_img.size());
  
  string new_imagename = "./new_img/";
  new_imagename += basename(string(imagename));

  if (imwrite(new_imagename, dst_img)) {
    cout << "imwrite:" << new_imagename << " ... success" << endl;
  } else {
    cout << "imwrite:" << new_imagename << " ... failure" << endl;
  }

  return 0;
}
