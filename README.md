## GETTING STARTED

```bash
$ git clone git@github.com:wantedly/manga.git && cd manga
$ make
$ ./manga (入力画像ファイル名) (出力画像ファイル名) (gray or color指定) (重ねたい効果音画像等)
```

## REQUIREMENTS

### pkg-config

- Makefileに使用 

### opencv 

- ubuntu 12.04、opencv 2.3.1-7 で動作確認済み。(apt-get install libcv-devで入るデフォルトのversion)
- OSX 10.9.2、opencv 2.4.8 で動作確認済み。(14/03/28時点でHomebrewでinstallできるデフォルトのversion)
