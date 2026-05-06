const int Next_BUTTON_PIN = 5;  // 下一首按鍵的接腳
const int PlayPause_BUTTON_PIN = 6;  // 播放/暫停 按鍵的接腳
const int Prev_BUTTON_PIN = 7;  // 上一首按鍵的接腳


int sensorPin = A5;    //音量控制類比輸入的PIN腳
int sensorValue = 0;  // 類比輸入得到的數值
int volume = 0;       //目前音量


#include <DFMiniMp3.h>  
//使用軟體Serial
#include <SoftwareSerial.h>  
SoftwareSerial mySerial(11, 10); // RX, TX

// 宣告 notify class
//
class Mp3Notify; 

// 定義類型，其中第一個參數為決定用硬體Serial或軟體Serail
//
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3;  //如果用UNO板就用軟體Serail

// 建立mp3物件
//
DfMp3 dfmp3(mySerial);

DfMp3_Status playOrPause;   // 建立播放狀態物件

// 實作notification的類別，可以在不同的事件中，寫入想要進行的動作
// 若沒有特別要進行的處理，這裡不用修改
class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError([[maybe_unused]] DfMp3& mp3, uint16_t errorCode)
  {
    // 錯誤訊息
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished([[maybe_unused]] DfMp3& mp3, [[maybe_unused]] DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnPlaySourceOnline([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved([[maybe_unused]] DfMp3& mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};

void setup() 
{
  Serial.begin(9600); //啟用監控視窗
  Serial.println("initializing...");

  pinMode(Next_BUTTON_PIN, INPUT_PULLUP); //設定按鈕的接腳為輸入
  pinMode(PlayPause_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(Prev_BUTTON_PIN, INPUT_PULLUP); 
  
  dfmp3.begin();  //開始使用DFPlayer模組

  //重置DFPlayer模組，會聽到"波"一聲
  dfmp3.reset();

  //音量控制，0~30
  uint16_t volume = dfmp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  dfmp3.setVolume(24);
  
  //取得所有MP3檔的總數
  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files ");
  Serial.println(count);

  Serial.println("Play Mp3...");

  // 播放MP3，參數就是第N首。如要播第一首MP3，參數就是1
  dfmp3.playMp3FolderTrack(1);  
}

void loop() 
{
  //音量控制，旋轉可變電阻，讀取類比值後，換算成音量，然後進行設定
  sensorValue = analogRead(sensorPin);   //讀取類比輸入的值會得到0~1023
  sensorValue = map(sensorValue,0,1023,0,30);  //將0~1023轉化成0~30音量
  if(volume != sensorValue){   //如果目前音量，不等於可變電阻設定的音量，就將音量設為新的值
    dfmp3.setVolume(sensorValue);
    volume = sensorValue;
    Serial.println(volume); //從監控視窗看到音量值
  }


  //判斷 播放/暫停 的狀態，如果正在播放，按了鈕就表示要暫停，除此之外，就接著播放
  if(digitalRead(PlayPause_BUTTON_PIN) == LOW){  // 如果按了鍵
    Serial.println("click Play/Pause..."); 
    playOrPause = dfmp3.getStatus();  //取得播放狀態
    if(playOrPause.state == DfMp3_StatusState_Playing ){   //判斷目前是不是播放中
      dfmp3.pause();  //暫停
      delay(500);     //delay半秒，以免操作者按住不放會一直重複操作
    }else{
      dfmp3.start();  //接著播放
      delay(500);
    }
  }

  //按下上一首按鍵的處理
  if(digitalRead(Prev_BUTTON_PIN) == LOW){          //如果按鍵按了
    dfmp3.prevTrack();
    delay(500);
  }

  //按下下一首按鍵的處理
  if(digitalRead(Next_BUTTON_PIN) == LOW){          //如果按鍵按了
    dfmp3.nextTrack();
    delay(500);
  }
  
}