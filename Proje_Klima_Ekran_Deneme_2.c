#include <Proje_Klima_Ekran_Deneme_2.h>
#include <benim_LCD.c>

#define YUKARI   PIN_B4
#define ASAGI    PIN_B5        
#define ENTER    PIN_B6                    
#define ON_OFF   PIN_B0    
 
#priority ext, rb, timer0  // KESME ONCELIGI  

// GLOBAL DEGISKENLER
int8 ana_sayac =  1; // MENU Seçimlerinde kullanýlan degiþken   
int8 tmp = 0;         // Menu Seçimlerinde kullanýlan Algoritma degiþkeni   
int8 sayfa = 0; 
int8 lcd_ust_limit = 4;
int8 sicaklik = 25;
int1 toggle = 0; 

// FONKSIYON ORNEKLERI
void ana_menu(int8 liste_secim);
void ok_isareti(int8 konum);             
void fan_ayar_fonksiyonu_menu(); 
void sicaklik_menu(void);
void saat_tarih_menu(void);
void fan_menu(void);
void mod_menu(void);
void menu_secim(void);
void sicaklik_ayarla(void);
void geri_don(void);




struct{               
int1 RBIF;     // RB Portunun Deðiþim Bayragý biti        
int1 INTF;     // RB0 External Kesme Bayragý biti
int1 TMR0IF;   // TMR0 Kesme Bayragý      
int1 RBIE;     // RB Portu Kesme Aktif biti
int1 INTE;     // RB0 Pin Kesme Aktif biti                             
int1 TMR0IE;   // TMR0 Kesme Aktif/Pasif              
int1 PEIE;     // Çevresel Kesmeler Aktif                    
int1 GIE;      // Global Kesme Aktif     
}INTCON;  
                                       
#byte INTCON  =  0x8B
#byte T2CON   =  0x12   //  Timer 2 Kontrol Register
#byte PR2     =  0x92   //  PR2 Registerý TMR2 ile karþýlaþtýrýlýr                       
#byte TMR2    =  0x11   //  TMR2 Registerý PR2 ile karþýlaþtýrýlýr 
#byte PIR1    =  0x0C   //  Timer 2 Kesmede kullanýlmasý için     
#byte PIE1    =  0x8C   //  Timer 2 Kesmede kullanýlmasý için                                   
#byte CCP1CON =  0x17   //  CCP Kontrol Registerý PWM Moduna Ayarlanacak Main'de 
#byte CCPR1L  =  0x15                                                          
#byte CCPR1H  =  0x16                                                         
#byte TRISA   =  0x85                
#byte TRISB   =  0x86                            
#byte TRISC   =  0x87   
#byte TRISD   =  0x88
#byte TRISE   =  0x89                    
#byte PORTA   =  0x05
#byte PORTB   =  0x06         
#byte PORTC   =  0x07
#byte PORTD   =  0x08 
#byte PORTE   =  0x09   

                           
#INT_RB                   
void RB_Port_degisim_aktif(void){       

                                        
   if(!input(YUKARI)){    
        delay_ms(20);
        ana_sayac++;  
   }                                      
   if(!input(ASAGI)){
        delay_ms(20);
        ana_sayac--;            
   }
   
   
   if(ana_sayac > lcd_ust_limit || ana_sayac < 1)                                                  
      ana_sayac = tmp;
                              
   if(!input(ENTER)){
  
       menu_secim();  
       ana_sayac = 1; 
       tmp = 0;         

   }             
                 
   if(tmp != ana_sayac && sayfa == 0){                                                                             
                                        
      ana_menu(ana_sayac);
      ok_isareti(ana_sayac);                            
      tmp = ana_sayac;        
   
   }
   
   if(tmp != ana_sayac && sayfa == 1){
      
      sicaklik_menu();
      ok_isareti(ana_sayac);
      tmp = ana_sayac;

   }
   
   if(tmp != ana_sayac && sayfa == 2){
   
              
   
   }
                              
                           
   INTCON.RBIF = 0;  
}                                      
// ÞABLON
void menu_secim(){      
                                 
   if(sayfa == 0){   
      switch(ana_sayac){
             
          case 1:
             sicaklik_menu();    break; 
          case 2:               
             fan_menu();         break;    
          case 3:              
             mod_menu();         break;          
          case 4:             
             saat_tarih_menu();  break;   
          default:              
             break;    
      } 
   }  
   else if(sayfa == 1){
        
    switch(ana_sayac){
             
          case 1:
             sicaklik_ayarla();    break;              
          case 2:               
             geri_don();           break;      
          default:              
             break;    
      } 
                                   
   }
   else if(sayfa == 2)
   {
         
         sicaklik = ana_sayac;    
         
         
   }
     
}
void geri_don(void){

   if(sayfa>1)
      sayfa--;             

}
void ana_menu(int8 liste_secim)                    
{                                                              
                                               
   if(liste_secim<=2){    
     lcd_temizle();                           
     printf(lcd_veri," Sicaklik Ayari"); 
     lcd_komut(0xC0); // Ýmleç ikinci satýr
     printf(lcd_veri," Fan Ayari");                     
                                              
   }
   else if(liste_secim>2){
      lcd_temizle();                         
      printf(lcd_veri," Modlar");               
      lcd_komut(0xC0); 
      printf(lcd_veri," Saat ve Tarih");                
   }
   imlec(2,15);                                                  
   printf(lcd_veri," %d",liste_secim);                                
    
}                            

void sicaklik_menu(void){
   
   sayfa = 1;
   lcd_ust_limit = 2; 
   lcd_temizle();
   imlec(1,1);
   printf(lcd_veri,"Sicaklik :");
   lcd_komut(0xC0);
   printf(lcd_veri,"Geri Gel");
   imlec(2,15);                                                  
   printf(lcd_veri," %d",ana_sayac);
   
                                                     
}                                                               
void sicaklik_ayarla(void){
     
    lcd_temizle(); 
    sayfa = 2;
    printf(lcd_veri,"Sicaklik : %d",sicaklik);
    
    
}

void fan_menu(void){



}


void mod_menu(void){


}

void saat_tarih_menu(void){



}

void ok_isareti(int8 konum){

   if(konum%2 == 0)
      konum = 2;                             
   else
      konum = 1;
   imlec(konum,1);
   printf(lcd_veri,">");
   
}          
          
          
          
              
          
          
void main()
{   
                                
   TRISA   =  0x00;                 
   PORTA   =  0x00;                  
   TRISB   =  0xFF;                                              
   PORTB   =  0x00;         
   TRISC   =  0x00;                     
   PORTC   =  0x00;
              
   lcd_init();         // LCD Baþlat                   
   ana_menu(1);        // LCD ilk menü yazýlmasý için               
   INTCON  =  0x98;    // INTCON REGISTER Global, B0 ve RB Port Kesme Aktif
 

   while(TRUE)
   {                     
      
   }

}
