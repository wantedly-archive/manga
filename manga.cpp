// main.cpp
#include <iostream>
#include <string>
#include "unistd.h"
#include <opencv2/opencv.hpp>
#include <opencv2/legacy/legacy.hpp>

using namespace std;
using namespace cv;

string dir = "/usr/local/var/www/manga"; // 実行ファイルおよびmaterialフォルダがある場所

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
  string imagename = argc > 1 ? argv[1] : dir + "/lena/lena.png";
  Mat src_img = imread(imagename);
  if (!src_img.data) {
    cout << "source file not found" << endl;
    return -1;
  }

  // 一度2の階乗にリサイズ(cvPyrSegmentationが2の階乗のサイズしか受け付けない)
  Size src_size = src_img.size();
  Mat resize_img(AlignPow2(src_size.height), AlignPow2(src_size.width), CV_64FC(3));
  resize(src_img, resize_img, resize_img.size());
  src_img.release();

  // convert color image to grayscale
  Mat gray_img;
  cvtColor(resize_img, gray_img, CV_BGR2GRAY);

  // 適応的に二値化(輪郭抽出)
  Mat bin_img;
  adaptiveThreshold(gray_img, bin_img, 255,
    CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 10);
  gray_img.release();

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
  pyr_img.release();
  
  // 画像を重ねる
  Mat and_img;
  bitwise_and(bin_img, gray_pyr_img, and_img);
  bin_img.release();
  gray_pyr_img.release();
  
  // src_imgのサイズに戻す
  Mat dst_img;
  resize(and_img, dst_img, src_size);
  and_img.release();

  // トーンを貼る用の領域を計算
  Mat mask_img;
  threshold(dst_img, mask_img, 180, 255, THRESH_BINARY);
  Mat min_mask_img;
  threshold(dst_img, min_mask_img, 80, 255, THRESH_BINARY);
  bitwise_not(min_mask_img, min_mask_img);
  add(mask_img, min_mask_img, mask_img);
  min_mask_img.release();
 
  // load tone image
  string tone_imagename = dir + "/material/tone.png";
  Mat tone_img = imread(tone_imagename);
  if (!tone_img.data) {
    cout << "tone file not found" << endl;
    return -1;
  }

  Mat gray_tone_img;
  cvtColor(tone_img, gray_tone_img, CV_BGR2GRAY);
  Mat resize_tone_img;
  resize(gray_tone_img, resize_tone_img, src_size);

  // toneの上に画像を重ねる
  dst_img.copyTo(resize_tone_img, mask_img);
  
  // load sound image
  string sound_imagename = dir + "/material/0078_gogo_jo.png";
  Mat sound_img = imread(sound_imagename, /* 0はgray scaleでの読み込み */ 0);
  if (!sound_img.data) {
    cout << "sound file not found" << endl;
    return -1;
  }
  
  Mat resize_sound_img;
  resize(sound_img, resize_sound_img, src_size);
  sound_img.release();
 
  Mat mask_sound_img;
  bitwise_not(resize_sound_img, mask_sound_img);

  // 音響イメージを重ねる
  resize_sound_img.copyTo(resize_tone_img, mask_sound_img);
  
  // new_imageファイルを保存
  string new_imagename = argc > 2 ? argv[2] : dir + "/new_" + basename(imagename);
  if (imwrite(new_imagename, resize_tone_img)) {
    cout << "imwrite:" << new_imagename << " ... success" << endl;
  } else {
    cout << "imwrite:" << new_imagename << " ... failure" << endl;
  }

  // namedWindow("Display Window", WINDOW_AUTOSIZE);
  // imshow("Display Window", resize_tone_img);
  // waitKey(0);

  return 0;
}
