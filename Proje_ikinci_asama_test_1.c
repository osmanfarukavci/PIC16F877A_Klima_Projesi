  
#include <16F877A.h> 
#device ADC=10                             
#use delay(crystal=4MHz)
#fuses NOWDT
#fuses XT,NOBROWNOUT,NOLVP         
#priority ext,rb,timer1
                                          
#define Yukari pin_b4                  //  YUKARI TUÞU PIN B4                 
#define Asagi  pin_b5                  //  AÞAGI  TUÞU PIN B5                    
#define enter  pin_b6                  //  ENTER  TUÞU PIN B6 
#define ee_use_portd_lcd      TRUE     //  LCD D PORTUNA BAÐLI
                                                               
                                                    

struct{         
int1 C;
int1 DC;
int1 Z;
int1 PD;                           
int1 TO;
int1 RP1;
int1 RP0;              
int1 IRP;
}OPTION_REG;
#byte OPTION_REG = 0x03 

                                                                                                                 
//EEPROM ADRESLERÝ                            
#define EEPROM                  0 
#define ee_sayfa                1 
#define ee_sicaklik             2                         
#define ee_fan_hiz_kademe       3                           
#define ee_gun                  5            
#define ee_ay                   6
#define ee_yil                  7             
#define ee_saniye               8
#define ee_dakika               9
#define ee_saat                 10                                      
#define ee_sicaklik_durum       11                                               
       
#include <lcd.c>  // LCD KÜTÜPHANESÝ EKLENDÝ                        
                                                     
#use fast_io(ALL) // TRISLERÝ KENDÝMÝZ AYARLIYORUZ                            


// FONKSIYON ORNEKLERI                           
void ana_menu(void);             // ANA MENUYU LÝSTELER VE SECÝM YAPILIR                          
void ok_isareti(int8 konum);     // TUM MENULERDE SECIM YAPIMINI SAGLAR                                                                      
void sicaklik_menu(void);        // SICAKLIK AYARININ YAPILDIÐI MENU            
void sicaklik_ayar(void);        // SICAKLIK AYARLANIR
void fan_menu(void);             // FAN AYAR MENU
void fan_kademe_ayar(void);      // FAN KADEMESÝNÝ AYARLANIR 0 - 5 OTOMATIK TE ETKÝ ETMEZ
void ekran_temizle(void);        // GENEL AMAÇLI LCD EKRANINI TAMAMEN SÝLER                          
void ana_ekrana_don(void);       // ANA MENUYE GERÝ DÖNDÜRÜR
void mod_menu(void);             // MOD SECIMININ YAPILDIÐI MENU                  
void mod_secim(void);            // MOD SEÇÝLÝR SICAK VEYA SOGUK
#separate                        // ROM u verimli kullanmak için. "Fonksiyon kopyasý oluþturulmadan"
void tarih_ve_saat_menu(void);   // TARÝH VE SAAT AYARLARI MENUSU                                                               
#separate                                                                                      
void tarih_ayar(void);           // TARÝH AYARLANIR 
void saat_ayar(void);            // SAAT  AYARLANIR                 
void durum_ekrani(void);         // TUM VERILERI GOSTEREN EKRAN // SICAKLIK FAN SAAT HATA                  
void menu_sablon(void);          // MENULERIN ULASIMI BU FONK DAN SAGLANIR SAYFA DEGISKENINE GORE CALISIR                    
void adc_calistir(void);         // ADC DEN SICAKLIK OLCUMU YAPILIR 
void eeprom_yaz(void);           // ELEKTRIK VeYA RESET DURUMUNDA EEPROMA VERI YAZMADA KULLANILIR 
void eeprom_cikart(void);        // ISLEMCI EKSTRA KESINTIDEN ÇIKTIGINDA EEPROMDAN VERIYI ALIR
int16 kontrol(int8 motor_kademe); // KLIMA KONTROL FONKSIYONU FAN, KLIMA MOTOR VE ISITICI TEL KONTROL
void mod_oto_man(void);           // OTOMATIK / MANUEL MOD SECIMI YAPILIR OTOMATIKTE FAN OTOMATIK ÇALIÞIR

// DEGISKENLER
int16 ana_sayac = 1;              // SAYFALARDA SECIM ISLEMLERI VE AYARLAMALARIN KULLANILDIÐI SAYACTIR                  
int16 ana_sayac_old = 1;          // OLASI HATALARDA ISTENILEN SAYAC DEGERINI TUTAR        
int8 sayfa = 10;                  // MENULERDE GEZINMEYI SAÐLAYAN SAYFA DEGISKENI                    
int1 enter_toggel = 0;            // ENTER TUÞUNA BASILDIÐINI ALGILAMAMIZI SAÐLAR            
int16 sayfa_limit = 4;            // SAYFA LÝMÝTÝ MENULERDEKÝ 2X16 EKRANA GORE SAYFA LÝMÝTLERÝNÝ BELÝRLER VE FARKLI AYARLARDA SAYFA LÝMÝTÝNÝ BELÝRLER        
int8 sicaklik = 25;               // ISTENILEN SICAKLIKTIR     
int1 sicaklik_mod = 0;            // 0 Soguk / 1 Sicak
int8 sicaklik_durum = 0;          // SENSORDEN OLCULEN SICAKLIKTIR
int8 fan_hiz_kademe = 0;          // FAN HIZ KADEMESI 5 KADEMEDE VEYA OTOMATIK MODDA CALISIR
int16 sayfa_alt_limit = 1;        // SAYFA LÝMÝTÝN ALT LÝMÝTÝDÝR
int8 gun = 1;                     // TARIH VE SAAT DEGISKENLERI
int8 ay  = 1;                                      
int16 yil = 2023;
int8 saniye = 0;
int8 dakika = 0;
int8 saat = 12;                    
int8 i = 0;                       
int8 tmr1_kesme = 0;              // TMR1 KESMESÝNE GIRDIKCE ARTAR                    
int8 ekran_reset_sayac = 0;       // BELÝRLÝ BÝR SÜREYÝ TUTAR VE BU SUREYE GORE DURUM EKRANI GELÝR
unsigned long int adc_kanal;      // ADC OLCUMU
float voltaj ;                    // ADC DEN OLCUME GORE GERÝLÝM DEGÝSKENÝ
int1 oto_mod = 0;                 // OTOMATIK / MANUEL MOD
int1 klima = 1;                   // KLIMA ACIK / KAPALI          
int8 hata_kod = 0;   /*           // HATA KODLARI               


KOD 1 SICAKLIK SENSOR TAKILI DEGIL
KOD 2
KOD 3                 
KOD 4

*/                            
#int_ext         
void klima_acma_kapama(void){    
                   
klima =!klima;
kontrol(0);
durum_ekrani(); 
}                            

#int_timer1
void timer1_kesmesi(void){                        
   set_timer1(63036);
   tmr1_kesme++;
                       
   if(tmr1_kesme==50){
      adc_calistir();
      saniye++;
      tmr1_kesme = 0;
         if(sayfa != 10 ){
         ekran_reset_sayac++;                       
         }             
         if(sayfa == 4 && ana_sayac <= 2){   
            lcd_gotoxy(7,2);                 
            printf(lcd_putc,"%02d:%02d:%02d"saat,dakika,saniye); 
            
         }
   }                                                   
   if(saniye > 59){ 
      dakika++;                                       
      saniye = 0;                         
      if(sayfa == 10){                                                
      sayfa = 10;         
      durum_ekrani();           
      }    
   }
   if(dakika > 59){
      saat++;                         
      dakika = 0;                          
      
   }
   if(saat > 23){                                   
      gun++;        
      saat = 0;          
   }        
   if(gun == 30){
      ay++;
      gun = 0; 
                                
   }                         
   if(ay == 12){                          
      yil++;
      ay = 0;
                           
   }    
   if(ekran_reset_sayac == 10){
      sayfa = 10;
      durum_ekrani();                  
      ekran_reset_sayac = 0;
       
   }                             
   restart_wdt();               
                                                                                           
}                               
                           
                                                        
#int_rb           
void B_port_degisim_kesmesi(){            

   ekran_reset_sayac = 0;  // Durum Ekranýna geçmemesi için
   if(klima){
   if(!input(Yukari)){
     ana_sayac++;
     delay_ms(20);                                                                           
    
   }
   
   if(!input(Asagi)){
     ana_sayac--;                                             
     delay_ms(20);               
   }                                             
                
   if(!input(enter)){
      enter_toggel = 1;                                                                                       
   }
                  
   if(ana_sayac < sayfa_alt_limit || ana_sayac > sayfa_limit)  
         ana_sayac = ana_sayac_old;  
                                        
   menu_sablon(); 
   output_toggle(pin_c0); 
   }
   restart_wdt();                           
                               
                                    
   enter_toggel = 0;     
}                             

void menu_sablon(void){
              
     switch(sayfa){
      case 0:                      
         ana_menu();  
         ok_isareti(ana_sayac);
         break;                                        
      case 1:              //   SICAKLIK MENU 
         sicaklik_menu();
         ok_isareti(ana_sayac);
         break;          
      case 2:          
         fan_menu();       //   FAN     MENU
         ok_isareti(ana_sayac);
         break;                            
      case 3:              //   MOD     MENU
         mod_menu();
         ok_isareti(ana_sayac);
         break;                                                 
      case 4:              //   TARÝH SAAAT MENU
         tarih_ve_saat_menu();
         ok_isareti(ana_sayac);
         break;        
      case 5:              // Sýcaklik Ayari Fonksiyonu      
         sicaklik_ayar();  
         break;           
      case 6:              // Fan Kademe Ayari Fonksiyonu                
         fan_kademe_ayar();      
         break;
      case 7:              // Modlar 
         mod_secim();
         ok_isareti(ana_sayac);   
         break;                                             
      case 8:              // Tarih Ayari Fonksiyonu
         tarih_ayar();   
         break;
      case 9:              // Saat Ayari Fonksiyonu
         saat_ayar(); 
         break;
      case 10:
         durum_ekrani();
         break;
      case 11:
         mod_oto_man(); // OTOMATIK MANUEL MOD SEÇÝMÝ 
         ok_isareti(ana_sayac);
         break;
      default:                                     
         sayfa = 0;
         break;                                             
   }                       
}
 
void ana_menu(void){          
   
  sayfa_limit = 4;
  sayfa_alt_limit = 1;
  
  if(ana_sayac<=2){    
     ekran_temizle();                                     
     printf(lcd_putc," Sicaklik");
     lcd_gotoxy(2,2);                      
     printf(lcd_putc,"Fan"); 
   }
   else if(ana_sayac>2 && ana_sayac<=4){
     ekran_temizle();
     printf(lcd_putc," Mod");
     lcd_gotoxy(2,2);                  
     printf(lcd_putc,"Tarih ve Saat");
                                                       
   }                                                
   lcd_gotoxy(15,1);
   printf(lcd_putc,"%ld",ana_sayac);
   
   if(enter_toggel){       
                                                  
       sayfa = ana_sayac;        
       ana_sayac = 1;
       ana_sayac_old = 1;
   }                  
   
} 

void sicaklik_menu(void){                   
   
   sayfa_limit = 2;                       
   ekran_temizle();                                     
   printf(lcd_putc," Sicaklik Ayari"); 
   lcd_gotoxy(2,2);
   printf(lcd_putc,"Geri");                     
   lcd_gotoxy(14,2);                
   printf(lcd_putc,"%02d",sicaklik);
   lcd_putc(223);     // sicaklik Derece isareti   
   if(enter_toggel){    
     if(ana_sayac == 1)                            
     {     
          sayfa = 5;           
          sayfa_limit = 60;         
          ana_sayac = sicaklik;       
     
     }                                     
     else if(ana_sayac == 2)
     {
       ana_ekrana_don();      
     }
      
       
   } 


}

void sicaklik_ayar(void){

   
   ekran_temizle();      
   printf(lcd_putc," Sicaklik Ayari :\n %02d",sicaklik);
   sicaklik = ana_sayac; 
   if(enter_toggel){                
       
      sayfa = 1;
      ana_sayac = 1;
   }
   
} 

void fan_menu(void){                
     
    ekran_temizle();
    sayfa = 2;           
    sayfa_limit = 2;    
    printf(lcd_putc," Fan Kademe Ayari"); 
    lcd_gotoxy(2,2);
    printf(lcd_putc,"Geri");
    lcd_gotoxy(8,2);
    printf(lcd_putc,"%d.Kademe",fan_hiz_kademe);
    if(enter_toggel){
      if(ana_sayac == 1){
          sayfa = 6;
          sayfa_limit = 5;  
          sayfa_alt_limit = 0;  
          ana_sayac = fan_hiz_kademe;
      
      }
      else if(ana_sayac == 2){
          
          ana_ekrana_don();
          
      } 
     
    
    }  
                                                  
}

void fan_kademe_ayar(void){ 
   
   ekran_temizle();          
   printf(lcd_putc," FAN KADEMESI : \n%d",fan_hiz_kademe);
   fan_hiz_kademe = ana_sayac;
   kontrol(fan_hiz_kademe);
   if(enter_toggel){
       sayfa = 2;
       ana_sayac = 1;
       sayfa_alt_limit = 1;
   }
}

void mod_oto_man(){

    ekran_temizle();                 
    printf(lcd_putc," OTOMATIK");
    lcd_gotoxy(2,2);         
    printf(lcd_putc,"MANUEL");
    if(enter_toggel){
         if(ana_sayac == 1)
            oto_mod = 1;
         else
            oto_mod = 0;
         sayfa = 3; 
    
    }


}                                          
void mod_menu(void){                               
                    
   sayfa = 3;
   sayfa_limit = 3;
   if(ana_sayac <= 2){           
                
      ekran_temizle();   
      printf(lcd_putc," Mod SECIMI YAP :"); 
      lcd_gotoxy(2,2);                         
      printf(lcd_putc,"DUZ / OTOMATIK ");                                       
   } 
   else if(ana_sayac >=3)
   {
      ekran_temizle();   
      printf(lcd_putc," Geri");     
   }                            
   if(enter_toggel){
      if(ana_sayac == 1){
          sayfa = 7;
          sayfa_limit = 2;
                                        
      }
      else if(ana_sayac == 2){
            
           sayfa = 11;
           sayfa_limit = 2;
           ana_sayac = 1;
      
      }
      else if(ana_sayac == 3){
      
          ana_ekrana_don();
          
      }
                     
    }   
                         

}
                                   
void mod_secim(void){
                                
     ekran_temizle(); 
     printf(lcd_putc," SICAK");
     lcd_gotoxy(2,2); 
     printf(lcd_putc,"SOGUK"); 
     if(enter_toggel)
     {
         if(ana_sayac == 1)
            sicaklik_mod = 1;
         else                             
            sicaklik_mod = 0;
         sayfa = 3; 
     }                                     
} 

#separate 
void tarih_ve_saat_menu(){
   ok_isareti(ana_sayac);                                 
   sayfa = 4;
   sayfa_limit = 3;  
   if(ana_sayac <= 2){             
      ekran_temizle();
      printf(lcd_putc," Tarih:%02d/%02d/%02ld",gun,ay,yil); 
      lcd_gotoxy(2,2); 
      printf(lcd_putc,"SAAT:%02d:%02d:%02d"saat,dakika,saniye);                             
                   
   }                             
   else if(ana_sayac <=3)           
   {                          
      ekran_temizle();   
      printf(lcd_putc," Geri");     
   }
   if(enter_toggel){
      
      if(ana_sayac == 1){                 
          sayfa = 8;                            
      }
      else if(ana_sayac == 2){
          sayfa = 9;      
      }
      else if(ana_sayac == 3){ 
       ana_ekrana_don();
      }
   
   
   }                                
                    
}
                           
#separate                       
void tarih_ayar(){
    
    if(enter_toggel){                  
      i++;    
      ana_sayac = 1; 
    }                               
    if(i == 0)
    {
    ekran_temizle();
    sayfa_limit = 30;
    sayfa_alt_limit = 0;    
    printf(lcd_putc," GUN :%02d",gun);
    gun = ana_sayac;   
    }
    else if(i == 1){
    ekran_temizle();
    sayfa_limit = 12;       
    sayfa_alt_limit = 1;
    printf(lcd_putc," AY : %02d",ay);
    ay = ana_sayac;         
                           
    }
    else if(i == 2){   // YIL BAÞLANGIÇ AYARI ÝÇÝN Basit bir hile yapýlmýþtýr
      ana_sayac = 2023;
      i++;
    }
    if(i == 3){
    ekran_temizle();
    sayfa_limit = 3000;                   
    sayfa_alt_limit = 2000;
    ana_sayac_old = 2023;     
    printf(lcd_putc," YIL : %ld",yil);
    yil = ana_sayac;                
                                        
    }                                                                                            
    else if(i == 4)  
    {         
      sayfa = 4;
      i = 0;
      sayfa_limit = 4;       
      sayfa_alt_limit = 1;
      ana_sayac = 1;                                                            
      ana_sayac_old = 1;
    }                          
                          
}
                       
void saat_ayar(){                     
    if(enter_toggel){
      i++;    
      ana_sayac = 0;                                       
    }                               
    if(i == 0)
    {
    ekran_temizle();           
    sayfa_limit = 23;                             
    sayfa_alt_limit = 0;           
    printf(lcd_putc," Saat :%02d",saat);
    saat = ana_sayac;     
    }                                                               
    else if(i == 1){       
    ekran_temizle();                                                                                                                   
    sayfa_limit = 59;       
    sayfa_alt_limit = 0;                                                              
    printf(lcd_putc," Dakika : %02d",dakika); 
    dakika = ana_sayac;         
                           
    }                                                                                                               
    else if(i == 2)                                    
    {                                                                   
      sayfa = 4;
      i = 0;
      sayfa_limit = 4;       
      sayfa_alt_limit = 1;
      ana_sayac = 1;                                                            
      ana_sayac_old = 1;                   
    } 

}  

void ok_isareti(int8 konum){                                             
     if(konum%2 == 0)
      konum = 2;
     else
      konum = 1;
     lcd_gotoxy(1,konum);          
     printf(lcd_putc,">");                                               

}

void ekran_temizle(){
   lcd_send_byte(0,0x01);
}

void ana_ekrana_don(void){                     
   sayfa = 0;
   ana_sayac = 1;    
}
                                  
void durum_ekrani(){         
   
                                                                           
   ekran_temizle();
   if(klima){ 
   lcd_gotoxy(1,1); 
   printf(lcd_putc," %02d",sicaklik_durum); 
   lcd_putc(223);
   if(sicaklik_mod)
         printf(lcd_putc," (+)"); 
   else                                          
         printf(lcd_putc," (-)");
   if(oto_mod)             
         printf(lcd_putc," O");
   else
        printf(lcd_putc," M");                   
   lcd_gotoxy(1,2);
   printf(lcd_putc,"Saat %02d:%02d",saat,dakika);            
   printf(lcd_putc," Fan %d",fan_hiz_kademe);
   }
   else{                            
   printf(lcd_putc,"Klima Kapali \n %02d",sicaklik_durum); 
   lcd_putc(223);                                
   }
   if(hata_kod > 0)
   {  
      lcd_gotoxy(14,1);
      printf(lcd_putc,"E%d",hata_kod);       
   }                  
   if(enter_toggel)
   {                                 
      sayfa = 0;                    
   }                                           
   ana_sayac = 1;
   ana_sayac_old = 1;   
                                        
}

              
                                         
void adc_calistir(){
       
     adc_kanal = read_adc();
     voltaj = (0.00488*adc_kanal)*1000;
     sicaklik_durum = (voltaj/10)+2;
     if(adc_kanal == 0)
         hata_kod = 1;
}
                            
void eeprom_yaz(){

   write_eeprom(ee_sayfa,sayfa);          
   write_eeprom(ee_fan_hiz_kademe,fan_hiz_kademe);
   write_eeprom(ee_gun,gun);
   write_eeprom(ee_ay,ay);
   write_eeprom(ee_yil,yil);
   write_eeprom(ee_dakika,dakika);
   write_eeprom(ee_saat,saat);      
   write_eeprom(ee_sicaklik,sicaklik);                    

}                                                            

void eeprom_cikart(){

   
   sayfa = read_eeprom(ee_sayfa);
   fan_hiz_kademe = read_eeprom(ee_fan_hiz_kademe);
   gun = read_eeprom(ee_gun);
   ay = read_eeprom(ee_ay);
   yil = read_eeprom(ee_yil); 
   dakika = read_eeprom(ee_dakika);
   saat = read_eeprom(ee_saat);
   sicaklik = read_eeprom(ee_sicaklik);
   

         
}   

int16 kontrol(int8 motor_kademe){ 
int16 motor_devir = 0; 
                                   
if(klima){                
   if(sicaklik_mod){ 
      output_high(pin_c0); // Rezistans Teli Açýk
      output_low(pin_c1);
      }
   else{                                         
      output_high(pin_c1); // Klima Motoru Açýk
      output_low(pin_c0);
   }
   if(oto_mod){
                 
    set_pwm1_duty((abs(sicaklik-sicaklik_durum))*100);   

}
else{ 
   switch(motor_kademe){
      case 0:
         set_pwm1_duty(0);         
         break;
      case 1:
         set_pwm1_duty(200);
         break;
      case 2:                         
         set_pwm1_duty(400);
         break;                                  
      case 3:
         set_pwm1_duty(600);                 
         break;                      
      case 4:
         set_pwm1_duty(800);
         break;
      case 5:
         set_pwm1_duty(1023);
         break;
      default:
         set_pwm1_duty(0);
         break;  
   } 
}
   
}

else{
 set_pwm1_duty(0);
 output_low(pin_c1);
 output_low(pin_c0); 
}
 
                 

return motor_devir;
}                    

void main()                      
{                   
   
   int saniye_old = 0;   
   
   set_tris_a(0xFF);
   set_tris_b(0xFF);
   set_tris_c(0x00);                      
   set_tris_d(0x00);
   output_c(0x00);
   lcd_init();               
   setup_WDT(WDT_2304MS);
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);
   setup_adc(adc_clock_div_32);
   setup_ccp1(CCP_PWM);
   set_pwm1_duty(0);
   setup_timer_2(T2_DIV_BY_4,249,1);
   setup_adc_ports(AN0);   
   set_timer1(63036);
   enable_interrupts(int_ext);  
   enable_interrupts(int_rb);
   enable_interrupts(int_timer1);
   enable_interrupts(GLOBAL);            
   sayfa = 10;                  
   voltaj = 0.00488*adc_kanal;                    
   set_adc_channel(0);        
   delay_us(20);
   
   if(restart_cause() == MCLR_FROM_RUN  || restart_cause() == NORMAL_POWER_UP){
                                                                            
       eeprom_cikart();
       ekran_temizle();                   
       printf(lcd_putc,"Resetleme");
       lcd_gotoxy(2,2);
       printf(lcd_putc,"%d",restart_cause()); // RESET NEDENÝ FROM RUN 27
                                              // RESEN NEDENÝ POWER UP
       delay_ms(1000);                                                                       
       durum_ekrani();
                                                        
   }                         
   if(restart_cause() ==  WDT_TIMEOUT){
                                             
      ekran_temizle();      
      printf(lcd_putc,"WDT TASMA HATASI"); 
       
   }                                                
                                                           
   while(TRUE)                                 
   {  
                                        
     if((saniye - saniye_old) == 10)
     {                                 
      eeprom_yaz();
      saniye_old = saniye;
      kontrol(fan_hiz_kademe);
     }
                                                                        
     
   }

}                                                                      
