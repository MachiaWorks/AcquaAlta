# AcquaAlta

### My home-brew MIDI Keyboard firmware「Acqua Alta」
#### by MachiaWorks

#### 1.概要

自作キーボードのファームウェアを新規に作成し、MIDIキーボード＋シーケンサとして使えるような形にしました。

作者（Machia）が腱鞘炎により楽器演奏はおろかマウスのコントロールすら困難になったけど、それでも作曲したいという欲求に対応する形で作ったファームウェアになります。

キーボードについては各自で買ってもらうとして、とりあえずプログラムを公開して他のキーボードでも使えるようにしておくのが公開の目的です。

#### 2.前提（用意するキーボード・パーツ・環境等）

* QMK firmwareは利用してません。
  コードを書いてFlashが必要になります。
* 自作キーボード「Nomu30」
  一応ソースコード上はNomu30を利用することを前提にピンやボタンを設定してますが、他のキーボードでも移植はそれほど難しくないものと考えます。
* ArduinoIDEで開発
* ProMicro(ATmega32U4, 5V/16MHz)を利用
* MsTimer2/MIDIUSBライブラリを利用してます。
  とりあえずATmegaファミリーであれば動くんじゃないかと思ってます。

#### 3.機能

* MIDIキーボード機能
  17鍵盤分、オクターブボタンでオクターブ移動およびVelocityの調整が可能
* CC送信機能
  Sustain/Expression/Modulation（おまけでVolumeとPan）のCCを送ることが可能
* シーケンサ機能
  手元でフレーズを作成して再生可能
  また、再生しつつキーボードで演奏も可能
  ただ、入力周りがまだバグってるので修正中

#### 4.作業手順

* キーボードを組む
  参考書籍がいっぱいあるので沼にハマってください。
* 下記ページを参考にファームウェアを反映させる
  [自作キーボードについて色々書く。 – MachiaWorks (machiaworx.net)](https://machiaworx.net/?p=1094)
* 動かす

#### 5.使い方

* 編集中

#### 6.ライセンス

MITライセンスに準拠します。

#### 7.謝辞

このプログラムは以下のページとソースコードを参考にして作っております。

[自作MIDIキーボード — Home-brew MIDI keyboard 0.05 ドキュメント (triring.net)](http://www.triring.net/gadget/Keyboard/midi_kbd/midi_kbd.html)

正直参考元と同様にビールウェア扱いでもいいかな？と考えましたが、かなり手を加えていることもあり一度定義した上でGithubにアップロードしておきます。
