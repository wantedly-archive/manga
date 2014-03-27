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

  // 一度2の階乗にリサイズ
  Size size = src_img.size();
  Mat resize_img(AlignPow2(size.height), AlignPow2(size.width), CV_64FC(3));
  resize(src_img, resize_img, resize_img.size());

  // convert color image to grayscale
  Mat gray_img;
  cvtColor(resize_img, gray_img, CV_BGR2GRAY);

  // 適応的に二値化(輪郭抽出)
  Mat bin_img;
  adaptiveThreshold(gray_img, bin_img, 255,
    CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 10);

  // cvPyrSegmentationを使うため、IplImageにキャスト
  IplImage cvresize = resize_img;
  IplImage *cvresize_p = &cvresize;
  
  // 領域補完
  CvMemStorage *storage = 0;
  CvSeq *comp = 0;
  storage = cvCreateMemStorage (0);
  IplImage *cvpyr = cvCloneImage (cvresize_p);
  cvPyrSegmentation (cvresize_p, cvpyr, storage, &comp, 4, 255.0, 50.0);
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

  // トーンを貼る用の領域を計算
  Mat mask_img;
  threshold(dst_img, mask_img, 180, 255, THRESH_BINARY);
  Mat min_mask_img;
  threshold(dst_img, min_mask_img, 80, 255, THRESH_BINARY);
  bitwise_not(min_mask_img, min_mask_img);
  add(mask_img, min_mask_img, mask_img);
 
  // load tone image
  const char* tone_imagename = "./img/tone.png";
  Mat tone_img = imread(tone_imagename);
  if (!tone_img.data) {
    cout << "file not found" << endl;
    return -1;
  }

  Mat gray_tone_img;
  cvtColor(tone_img, gray_tone_img, CV_BGR2GRAY);
  Mat resize_tone_img;
  resize(gray_tone_img, resize_tone_img, src_img.size());

  cout << "resize_tone_img size: " << resize_tone_img.size() << endl;
  cout << "dst_img size: " << dst_img.size() << endl;

  // toneを貼る
  dst_img.copyTo(resize_tone_img, mask_img);
  
  // new_imageファイルを保存
  string new_imagename = "./new_img/";
  new_imagename += basename(string(imagename));
  if (imwrite(new_imagename, resize_tone_img)) {
    cout << "imwrite:" << new_imagename << " ... success" << endl;
  } else {
    cout << "imwrite:" << new_imagename << " ... failure" << endl;
  }

  namedWindow("Display Window", WINDOW_AUTOSIZE);
  imshow("Display Window", dst_img);
  waitKey(0);

  return 0;
}
