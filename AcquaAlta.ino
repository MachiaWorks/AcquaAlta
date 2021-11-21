/*
  My home-brew MIDI Keyboard firmware「Aqua Arta.」
*/

#include "MIDIUSB.h"
#include <MsTimer2.h>

const int rowNum =  3;//キーボードの縦軸
const int colNum = 12;//キーボードの横軸
const int seq_lim = 16;//シーケンスのリミット
const int note_lim=17;//キーボード鍵盤のリミット
const byte key_in_buffer = 10;


// ピンの割当 for nomu30
const int rowPin[rowNum] = { 2, 3, 4} ;
//const int colPin[colNum] = { A3, A2, A1, A0, 15, 14,   16, 10, 8, 7, 6, 5 };
const int colPin[colNum] = { 5,6,7,8, A3, A2, A1, A0, 15,14,16,10 };

//CC番号の定義
const byte ctrlCc[5] = {64,//sustain
                        7,//MainVolume
                        11,//Expression
                        10,//PanPot
                        1//Modulation
};
const byte ctrlValue[12] = {0,12,24,36,48,64,
                            76,88,100,112,124,127};

//キーの機能割当
const byte keyFunctionMap[rowNum][colNum] = {
  /*
  - □ □ □ □ □ □ □ □ □ □ □
  ■ □ □ □ □ □ □ □ □ □  ■ - 
   ■ □ □ □ □ □ □ □ ■ - - - 
  */
  {0,2,4,0,    7,9,11,0,   14,16,0,0 },
  {1,3,5,6,    8,10,12,13,  15,17,22,0 },
  {27,18,19,20, 21,0,25,26,   24,0,0,0 }
};

/*
  0:なし
  1-17:鍵盤
  18/19:オクターブ変更
  20/21:Velocity変更
  22:シーケンス開始/停止
  23:シーケンス（現状）削除
  24:録音トグル
  25/26:シーケンスの移動
*/

// 押されたキーの情報を管理するための配列
bool currentState[rowNum][colNum];
bool beforeState[rowNum][colNum];
byte currentNote[rowNum][colNum];

byte keyCount[rowNum][colNum];


//利用変数。
int row, col;
byte note = 0;
byte oct_shift = 5;
byte vel_shift=10;
int velocity=100;

byte seq_on = 0;
byte before_seq_on=0;
byte seq_wait;

byte prev_seq = 0;
byte curr_seq = 0;
int bpm = 120;//60-240
bool rec_on = false;
bool shift_on = false;
byte send_cc_no= 64;

byte seq_count = 0;//const int seq_lim = 16;//シーケンスのリミット

//int holdMilliSec[note_lim];

//シーケンサ情報。17音ポリフォニックで持つ
byte seq_note[note_lim][seq_lim] = {
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },                            
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },//12
                           {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },
                            {255,255,255,255, 255,255,255,255, 
                            255,255,255,255, 255,255,255,255 },//17
                            };
//シーケンサのVelocity情報。17音ポリ想定
byte seq_vel[note_lim][seq_lim] = {
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },                        
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            {100,100,100,100, 100,100,100,100,
                            100,100,100,100, 100,100,100,100 },
                            };

//音を伸ばす対応のために確保した「今ならしてる音」を格納する配列
byte now_seq[note_lim] = {255,255,255,255,255, 255,255,255,255,255,
                          255,255,255,255,255, 255,255
                         };
                           
unsigned long prev=0,interval=0;

void setup() {
  // Serial.begin(115200);
  
  //デバッグ用のシリアル
  Serial.begin(31250);
  
  // 行単位の制御ピンの初期化
  for ( row = 0; row < rowNum; row++) {
    pinMode(rowPin[row], OUTPUT);
    delay(10);
    digitalWrite(rowPin[row], HIGH);
  }

  // 列情報読込みピンの初期化
  for ( col = 0; col < colNum; col++) {
    pinMode(colPin[col], INPUT_PULLUP);
  }
  
  // 読み取ったキーの情報を記録しておく配列を初期化する。
  for (row = 0; row < rowNum; row++) {
    for ( col = 0; col < colNum; col++) {
      currentState[row][col] = HIGH;
      beforeState[row][col] = HIGH;
      currentNote[rowNum][colNum] = 0;
      keyCount[row][col]=0;
    }
  }
  prev = millis();

  //タイマー起動(1秒に100/4=250回くらいのループ想定)
  MsTimer2::set(4, seqLoop); // 500ms period
  MsTimer2::start();
  
  interval = 100;   // 実行周期を設定
}

void seqLoop(){
  unsigned long curr = millis();
  

  //シーケンス処理
  if( seq_on ==1 ){
    //Serial.print("timer function : ");
    //Serial.print(curr-prev);
    if ((curr - prev) >= interval-24) {  // 前回実行時刻から実行周期以上経過していたら

      byte temp_curr_seq = (curr_seq+1>seq_count)?0:curr_seq+1;
      
      for( byte i=0; i<17; i++){
        if( seq_note[i][temp_curr_seq] != 255){
          // ノートOFFとONを発する
          noteOff(0, now_seq[i], seq_vel[i][curr_seq] );   // Channel 0, note, normal velocity
        }
      }
      MidiUSB.flush();

      if ((curr - prev) >= interval){
        // do periodic tasks            // 周期処理を実行
        prev = curr;                    // 前回実行時刻を現在時刻で更新
        
        Serial.print("timer update : ");
  
        //前シーケンスの情報持っておく（多分今のシーケンス情報だけでもいいと思うけど一応）
        prev_seq = curr_seq;
  
        //シーケンス加算
        curr_seq++;
  
        //加算したシーケンスが16以内に収まるように調整
        curr_seq = (curr_seq>seq_count)?0:curr_seq;
  
        for( byte i=0; i<17; i++){
          //255=OFFにする
          if( seq_note[i][curr_seq] != 255){
            noteOn(0, seq_note[i][curr_seq], seq_vel[i][curr_seq] );   // Channel 0, note, normal velocity
            now_seq[i] = seq_note[i][curr_seq];
          }
        }
        MidiUSB.flush();
        
        Serial.print("prev : ");
        Serial.println(prev_seq);
        Serial.print("seq update : ");
        Serial.println(curr_seq);
      }
    }
    //Serial.println("end:");
  }
  else{
    if( before_seq_on != seq_on){
      seqStop();
    }
  }

  before_seq_on = seq_on;
}

void seqStart(){
  //シーケンス開始。
  seq_on=1;
  curr_seq=0;

  //タイマーの値を入力する。
  prev = millis();
  //シーケンスの間隔を計算する。

  //60000(msec) ÷ BPM=4分音符の長さ
  //よって、更に4で割ればシーケンス（16ティック）の長さになる
  interval = 60000/bpm/4;   // 実行周期を設定
  Serial.println(interval);
  
  for( byte i=0; i<17; i++){
    if( seq_note[i][curr_seq] != 255 ){
      noteOn(0, seq_note[i][curr_seq], seq_vel[i][curr_seq]);   // Channel 0, note, normal velocity
      now_seq[i] = seq_note[i][curr_seq];
    }
  }
  MidiUSB.flush();
  Serial.print("sequence start: ");
  Serial.println(curr_seq);
}

void seqStop(){
  for( byte i=0; i<17; i++){
    for( byte j=0;j<16; j++)
    {

      if( now_seq[i] != 255 ){
        noteOff(0,now_seq[i] , seq_vel[i][j]);   // Channel 0, note, normal velocity
      }
    }
    MidiUSB.flush();
    now_seq[i] = 255;
    //delay(10);
  }
  MidiUSB.flush();
}

void loop() {
  // コントロールキーの読み取り
  row = rowNum;

  // 音階キーの読取りと発音
  for (row = 0; row < rowNum; row++) {

    //一度ピンを全部ローにする（差分を確認するのが目的じゃないかな）
    digitalWrite( rowPin[row], LOW );
    for ( col = 0; col < colNum; col++) {
      //
      currentState[row][col] = digitalRead(colPin[col]);
      if ( currentState[row][col] != beforeState[row][col] ) {
        
        Serial.print("key(");
        Serial.print(row);
        Serial.print(",");
        Serial.print(col);
        Serial.print(")");
        
        //high→Lowになった時のみ処理
        if ( currentState[row][col] == LOW && keyCount[row][col]>key_in_buffer ) {
          //note = noteKeyMap[row][col] + oct_shift;
          keyCount[row][col]=0;

          if( keyFunctionMap[row][col] != 0){
            /*
              {0,2,4,0,    7,9,11,0,   14,16,0,0 },
              {1,3,5,6,    8,10,12,13,  15,17,22,0 },
              {27,18,19,20, 21,0,25,26,   24,0,0,0 }

              0:割当なし
              1-17:鍵盤
              18/19:オクターブ変更
              20/21:Velocity変更
              22:シーケンス再生
              23:なし
              24:録音シフトキー
              25:シーケンス削除
              26:シーケンス増加
              27:シフトキー
              */
            //鍵盤演奏(func =1～17)
            if(keyFunctionMap[row][col] <=17){
              if(shift_on==true){
                //CC送信
                //controlChange(byte channel, byte control, byte value)
                if( keyFunctionMap[row][col] <=12){
                  //CC送信する
                  byte dat = 127/12*keyFunctionMap[row][col];
                  controlChange(0,send_cc_no,dat);
                }else{
                  //controlChange番号の変更
                  send_cc_no = ctrlCc[keyFunctionMap[row][col]-13];
                }
              }else{
                //鍵盤演奏
                NoteCalc();
                //note = keyFunctionMap[row][col]+oct_shift*12;
                note = (note>128)?127:note;
                
                velocity = 10*vel_shift;
                velocity = (velocity>128)?127:
                          velocity;
                          
                //シーケンス記録をおこなう
                if( rec_on == true){
                  seq_note[ keyFunctionMap[row][col]-1 ][seq_count] = note;
                  seq_vel[keyFunctionMap[row][col]-1][seq_count] = velocity;
                }
                //ノートのコントロールを行う。（再生）
                NoteControl(0,note, velocity);
              }
            }
            //オクターブ変更
            else if(keyFunctionMap[row][col] <=19){
              Serial.print("Octave : ");

              OctaveShift(keyFunctionMap[row][col]-18);
              Serial.print(oct_shift);
            }
            //Velocity変更
            else if(keyFunctionMap[row][col] <=21){
              Serial.print("Velocity : ");

              VelocityChange(keyFunctionMap[row][col]-20);
              Serial.print(vel_shift);
            }
            //シーケンス再生/停止
            else if(keyFunctionMap[row][col] == 22){
              Serial.print("Sequence : ");

              //1回押すごとに再生と停止を切り替える
              if( seq_on == 0){
                //Serial.print("start ");
                seq_on = 1;
                seqStart();
              }else{
                //Serial.print("stop ");
                seq_on = 0;
                //seqStop();
              }
            }/*
            else if(keyFunctionMap[row][col] == 23){
              Serial.print("AllStop : ");
              //暫定対応。
              midiEventPacket_t noteOff = {0x0B, 0xB0 | 0, 120, 0};
              MidiUSB.sendMIDI(noteOff);
            }*/            
            //録音キー
            else if(keyFunctionMap[row][col] == 24){
              rec_on=true;
            }
            //シーケンス削除キー
            else if(keyFunctionMap[row][col] == 25){
              if( rec_on == true){
                seqErase();
              }else{
              //今のカラム消去
                seqColumnErase(&seq_count);
              }
            }
            //次のシーケンス移行
            else if(keyFunctionMap[row][col] == 26){
              seq_count = (seq_count+1>=16)?15:seq_count+1;
            }
            //シフトキー
            else if(keyFunctionMap[row][col] == 27){
              shift_on=true;
            }
          }

          //今となっては不要かもの記述。
          currentNote[row][col] = note;

          //MIDI命令を送信する
          MidiUSB.flush();
          
          Serial.println(" Push!");
          //  Keyboard.press( keyMap[row][col] );
        } else {
          //鍵盤を離したときの処理
          if( keyFunctionMap[row][col] != 0 && keyCount[row][col]>key_in_buffer ){
            keyCount[row][col]=0;
            
            if(keyFunctionMap[row][col] <=17){
              //鍵盤については離すといずれのケースも音を止めるようにした。
              //if(shift_on==false)
              {
                NoteCalc();
                //note = keyFunctionMap[row][col]+oct_shift*12;
                note = (note>128)?127:note;
  
                velocity = 10*vel_shift;
                velocity = (velocity>128)?127:velocity;
                
                NoteControl(1,note, velocity);
              }
            }
            //録音キー離したら録音モード解除
            else if(keyFunctionMap[row][col] == 24){
              rec_on=false;
            }
            //シフトキー離したらシフト解除
            else if(keyFunctionMap[row][col] == 27){
              shift_on=false;
            }
          }

          MidiUSB.flush();
          Serial.println(" Release!");
          //  Keyboard.release( keyMap[row][col] );
        }        
        //
        beforeState[row][col] = currentState[row][col];
      }

      keyCount[row][col]= (keyCount[row][col]>=254)?254:keyCount[row][col]+1;
    }
    digitalWrite( rowPin[row], HIGH );
  }
}

void seqErase(){
  //
  for(byte i=0;i<17; i++){
    for( byte j=0; j<16; j++){
      seq_note[i][j] = 255;
      seq_vel[i][j] = 100;
    }
  }

  //カウンタも戻す。
  seq_count=0;
}

void seqColumnErase( byte *erase_col){
  for( byte i=0; i<17; i++){
      seq_note[i][*erase_col] = 255;
      seq_vel[i][*erase_col] = 100;
  }
}

void NoteCalc(){
  note = keyFunctionMap[row][col]+oct_shift*12-1;
}

void NoteControl( byte flag, byte note, byte velocity ){
  if( flag == 0){
      noteOn(0, note, velocity);   // Channel 0, note, normal velocity
  }
  else if( flag ==1 ){
      noteOff(0, note, velocity);   // Channel 0, note, normal velocity
  }
}

void OctaveShift( byte flag ){
  byte tmp_oct = oct_shift;
  tmp_oct = (flag ==1)?tmp_oct+1:
              (flag ==0)?tmp_oct-1:
              tmp_oct;

  oct_shift = (tmp_oct<1)?oct_shift:
              (tmp_oct>8)?oct_shift:
              tmp_oct;
}

void VelocityChange( byte flag ){
  byte tmp_vel = vel_shift;
  tmp_vel = (flag ==1)?tmp_vel+1:
              (flag ==0)?tmp_vel-1:
              tmp_vel;

//境界値条件
  vel_shift = (tmp_vel<1 )?vel_shift:
              (tmp_vel>11)?vel_shift:
              tmp_vel;
}


void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
