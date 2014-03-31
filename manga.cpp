// main.cpp
#include <fstream>
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

void file_copy(string src, string dst) {
  ifstream ifs(src);
  if (!ifs) {
    cout << "copy source file open error: " << src << '\n';
  }
  
  ofstream ofs(dst);
  if (!ofs) {
    cout << "copy out file open error: " << dst << '\n';
  }

  // コピー
  ofs << ifs.rdbuf() << flush;
}

int main(int argc, char* argv[]) {
  // load color image
  string imagename = argc > 1 ? argv[1] : dir + "/lena/lena.png";
  Mat src_img = imread(imagename);
  if (!src_img.data) {
    cout << "source file not found" << endl;
    string new_imagename = argc > 2 ? argv[2] : dir + "/new_" + basename(imagename);
    file_copy(imagename, new_imagename);
    return -1;
  }

  // 素材画像を grayscale に変換
  Mat gray_img;
  cvtColor(src_img, gray_img, CV_BGR2GRAY);

  // 適応的に二値化(輪郭抽出)
  Mat line_img;
  adaptiveThreshold(gray_img, line_img, 255,
    CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 10);
  gray_img.release();

  // 輪郭画像をRGBに戻す
  Mat color_line_img;
  cvtColor(line_img, color_line_img, CV_GRAY2BGR);
  
  // 一度2の階乗にリサイズ(cvPyrSegmentationが2の階乗のサイズしか受け付けない)
  Size src_size = src_img.size();
  Mat resize_img(AlignPow2(src_size.height), AlignPow2(src_size.width), CV_64FC(3));
  resize(src_img, resize_img, resize_img.size());
  src_img.release();

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

  // src_imgのサイズに戻す
  Mat resize_pyr_img;
  resize(pyr_img, resize_pyr_img, src_size);
  pyr_img.release();
  
  // 画像を重ねる
  Mat dst_img;
  bitwise_and(color_line_img, resize_pyr_img, dst_img);
  color_line_img.release();
  resize_pyr_img.release();
  
  // 音響イメージが題4引数として渡されていれば、読み込み
  if (argc > 4) {
    // load sound image
    string sound_imagename = argv[4];
    Mat sound_img = imread(sound_imagename);
    
    if (!sound_img.data) {
      cout << "sound file not found" << endl;
    } else {
      Mat resize_sound_img;
      resize(sound_img, resize_sound_img, src_size);
      sound_img.release();

      // 一度 grayscale に変換してから、マスクimage生成
      Mat gray_sound_img;
      cvtColor(resize_sound_img, gray_sound_img, CV_BGR2GRAY);
      Mat mask_sound_img;
      bitwise_not(gray_sound_img, mask_sound_img);

      // 音響イメージを重ねる
      resize_sound_img.copyTo(dst_img, mask_sound_img);
    }
  }

  // gray 文字列が第3引数として渡されていれば、grayに変換し、toneを使用
  if (argc > 3 && !strncmp(argv[3], "gray", 4)) {
    // dst_imgの grayscale を用意
    Mat gray_dst_img;
    cvtColor(dst_img, gray_dst_img, CV_BGR2GRAY);
 
    // load tone image
    string tone_imagename = dir + "/material/tone.png";
    Mat tone_img = imread(tone_imagename, /* 0はgray scaleでの読み込み */ 0);

    if (!tone_img.data) {
      cout << "tone file not found" << endl;
    } else {
      // トーンを貼る用の領域を計算
      Mat mask_img;
      threshold(gray_dst_img, mask_img, 180, 255, THRESH_BINARY);
      Mat min_mask_img;
      threshold(gray_dst_img, min_mask_img, 80, 255, THRESH_BINARY);
      bitwise_not(min_mask_img, min_mask_img);
      add(mask_img, min_mask_img, mask_img);
      min_mask_img.release();
      bitwise_not(mask_img, mask_img);
   
      // tone画像をresize
      Mat resize_tone_img;
      resize(tone_img, resize_tone_img, src_size);

      // tone画像を重ねる
      resize_tone_img.copyTo(gray_dst_img, mask_img);
    }

    dst_img = gray_dst_img;
  }
  
  // new_imageファイルを保存
  string new_imagename = argc > 2 ? argv[2] : dir + "/new_" + basename(imagename);
  if (imwrite(new_imagename, dst_img)) {
    cout << "imwrite:" << new_imagename << " ... success" << endl;
  } else {
    cout << "imwrite:" << new_imagename << " ... failure" << endl;
  }

  // namedWindow("Display Window", WINDOW_AUTOSIZE);
  // imshow("Display Window", dst_img);
  // waitKey(0);

  return 0;
}
