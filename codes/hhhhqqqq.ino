#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>


using namespace std;

//-----------------------------------------
#define RST_PIN  D3  //menjelaskan bahwa RST (reset) tersambung dengan pinout Digital 3 pada nodemcu
#define SS_PIN   D4  //menjelaskan bahwa ss/SDA tersambung dengan pinout Digital 4 pada nodemcu

//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;            //objek struktur key digunakan untuk menyimpan 6 byte key autentifikasi
MFRC522::StatusCode status;      
//-----------------------------------------

int blockangka = 2;  

byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
String nama_pada_kartu;
const String sheet_url = "https://script.google.com/macros/s/AKfycbwf-dxA7pDrbkWtNBZfKlNh4szOfmx2mu03XAJTtsXE/exec?name=";
/*-----------------------------------------
Dari kode dibawah dapat dijelaskan bahwa memiliki fungsi untuk menyambungkan NodeMcu ke Wifi dengan username_WIFI : "gavra ganteng"
dengan password : "hahahaha1"
*/

#define username_WIFI "gavra ganteng" 
#define PASSWORD_WIFI "hahahaha1"
//-----------------------------------------




/****************************-_************************************************************************
 * setup() fungsi
 ****************************************************************************************************/
void setup()
{
  //--------------------------------------------------
  
  Serial.begin(9600);
  
  
  //konektifitas WiFi 

  Serial.println();
  Serial.print("p");
  WiFi.begin(username_WIFI, PASSWORD_WIFI); // untuk menginisiasi pengaturan nama Wifi dan password wifi agar dapat tersambung ke wifi
  
  while (WiFi.status() != WL_CONNECTED){
    Serial.print("...");
    delay(200);
  }
// kode diatas berfungsi ketika status wifi tidak tersambung dengan NODEMCU maka akan melakukan perulangan mencetak string "..." setiap 0,2 detik  sekali dikarenakan terdapat delay(200); 

  Serial.println("");
  Serial.println("WiFi tersambung.");
  Serial.println("alamat IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

/*
Jika status wifi tersambung maka pada serial monitor akan mencetak kalimat "Wifi Tersambung."
beserta alat ip wifi
*/

  //--------------------------------------------------
  /* inisiasi SPI bus */
  SPI.begin();
  //--------------------------------------------------
}




/****************************************************************************************************
 * loop() function
 ****************************************************************************************************/
 void loop()
{
  //--------------------------------------------------
  /* menginisialisasi modul MFRC522  */
  mfrc522.PCD_Init();
  /* Cari kartu baru */
  /*digunakan untuk Setel ulang loop jika tidak ada kartu baru pada pembaca kartu RC522 */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  /* Pilih salah satu kartu */
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  /* Baca data dari blok yang sama */
  //--------------------------------------------------
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockangka, bacaBlockData);
/* Jika Anda ingin mencetak dump memori penuh, batalkan komentar pada baris berikutnya */
   //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
   /* Cetak data yang dibaca dari blok */
  Serial.println();
  Serial.print(F("data terakhir kartu RFID:"));
  Serial.print(blockangka);
  Serial.print(F(" --> "));
  for (int j=0 ; j<16 ; j++)
  {
    Serial.write(readBlockData[j]);
  }
  Serial.println();
  //--------------------------------------------------

  //--------------------------------------------------
  
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if (WiFi.status() == WL_CONNECTED) {
    //-------------------------------------------------------------------------------
    unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    //-------------------------------------------------------------------------------
    client->setFingerprint(fingerprint);
//  jika ingin mengabaikan sertifikat SSL
     // gunakan baris berikut sebagai gantinya:
     // klien->setInsecure();
    //-----------------------------------------------------------------
    nama_pada_kartu = sheet_url + String((char*)bacaBlockData);
    nama_pada_kartu.trim();
    Serial.println(nama_pada_kartu);
    //-----------------------------------------------------------------
    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    //-----------------------------------------------------------------

    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    if (https.begin(*client, (String)nama_pada_kartu)){
      //-----------------------------------------------------------------
      // cetak httpCode ke Serial
      Serial.print(F("[HTTPS] GET...\n"));
      // mulai koneksi dan kirim  data rfid
      int httpCode = https.GET();
      //-----------------------------------------------------------------
      // httpCode akan bernilai negatif bila error
      if (httpCode > 0) {
        // data rfid sudah terkirim dan server menanggapi 
        Serial.printf("Absensi Berhasil... code: %d\n", httpCode);
        // file ditemukan di server
      }
      //-----------------------------------------------------------------
      else 
      {Serial.printf("absensi gagal... , error: %s\n", https.errorToString(httpCode).c_str());}
      //-----------------------------------------------------------------
      https.end();
      delay(1000);
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    else {
      Serial.printf("tidak dapat tersambung dengan google drive \n");
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}




/****************************************************************************************************
 * ReadDataFromBlock() fungsi
 ****************************************************************************************************/
void ReadDataFromBlock(int blockangka, byte bacaBlockData[]) 
{ 
  //----------------------------------------------------------------------------
  /* mempersiapkan key untuk autentikasi */
  /* semua key akan diset menjadi  FFFFFFFFFFFFh reset chip pabrik */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------
  /* Authentifikasi menggunakan Key A atau B */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockangka, &key, &(mfrc522.uid));
  //----------------------------------------------------------------------------s
  if (status != MFRC522::STATUS_OK){
     Serial.print("autentikasi tidak berhasil: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
//Setelah dilakukan autentifikasi baru kita bisa membaca atau menulis  memory TAG / RFID Card  
  //----------------------------------------------------------------------------
  else {
    Serial.println("autentikasi berhasil");
  }
  //----------------------------------------------------------------------------
  /* Membaca Memori TAG /RFID Card */
  status = mfrc522.MIFARE_Read(blockangka, bacaBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("tidak dapat membaca kartu anda: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
     //kalau berhasl maka data hasil baca disimpan di array bernama buffer. sebelumnya kita deklarasikan dulu array buffer tsb.
  }
 
  
  //----------------------------------------------------------------------------
  else {
    Serial.println("......");  
  }
  //----------------------------------------------------------------------------
}