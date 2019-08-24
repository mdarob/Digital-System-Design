#include "derivative.h"

#define  LCD_DAT  PORTK    /* Port K drives LCD data pins, E, and RS */
#define  LCD_DIR  DDRK     /* Direction of LCD port */
#define  LCD_E    0x02     /* E signal */
#define  LCD_RS   0x01     /* RS signal */
#define  LCD_E_RS 0x03     /* assert both E and RS signal */
          
short push_counter; ; 
unsigned int counter;
int HiorLo_timer, HiorLo_sound; 
short HiCnt_timer, LoCnt_timer;
short HiCnt_sound, LoCnt_sound; 
int result;
int bounce;
int sound_counter;
short sound_case, sound_flag; 
int temp_d;
int position; 
int position_2;
char buffer[8];
char buffer_2[8]; 
int speed;
short direction; 
	      
short sound[3] = {48000,32000,24000};   

char mode[5][15] = {     

         "Pot Mode",
         "Temp Mode",
         "Light Mode",
         "Auto Mode",
         "Serial Mode"
};


char * clear = "                               ";         


void openSCI0(void)
{
     SCI0BDH = 0;
     SCI0BDL = 156;
     SCI0CR1 = 0x4C;
     SCI0CR2 = 0x0C;
}

int is_digit(int a){
   
    if( a >= 0x30  && a <= 0x39){
       
       return 1;
   
    }else{
    
       return 0; 
    
    } 
   
} 

void set_null(char *arr, int size){

   int i = 0; 
   
   for(i = 0; i < size; i++){
     
         arr++; 
         *arr = 0;
     
   }
   
} 


int length(char *arr, int size){
   
     int length = 0;
     while(*arr){
       
       arr++;
       length++; 
       
       if(length > size){
        
          return -1;
          
       }
       
     }
     
     return length; 
   
   
} 
void putch(char cx)
{
    while(!(SCI0SR1 & SCI0SR1_TDRE_MASK))
       continue;
    SCI0DRL = cx;

}

void putSCI(char *cptr)
{
    while(*cptr) {
        putch(*cptr);
        cptr++;
    }
}

void clearline(void){
   
   putch(0x0D);
   putSCI(clear);
  
   
}

void carriage_return(void){

     putch(0x0D);   
  
}
                         

void newline(void)
{
     putch(0x0D); 
     putch(0x0A); 
     putch(0x20); 
}

char getch(void)
{

     while(!(SCI0SR1 & SCI0SR1_RDRF_MASK) && (PIFH != 0x01)); //<- fix 
     return SCI0DRL;
}



void getstr(char *ptr)
{
       char cx;
       int i = 0;
       while ((cx = getch()) != 0x0D) {
       
               *ptr++ = cx;
               i++;
               putch(cx);
               if (cx == 0x08) { 
                       ptr--;
                       set_null(ptr,8);
               }
       }
       *ptr    = 0; 
       i++;
      
}


void SetClk8 (void) 
{  
    SYNR    = 0x02; 
    REFDV   = 0;    
    PLLCTL  = 0x60; 
    while(!(CRGFLG & CRGFLG_LOCK_MASK));
    CLKSEL  |= CLKSEL_PLLSEL_MASK; 
}

void delayby50us(int kk)
{
     int i;
     TIOS  |= 0x01;   		// enable OC0
     TC0 = TCNT + 1;  	// start one OC0 operation
     for (i = 0; i < kk; i++) 
     {
         while(!(TFLG1 & 0x01));
         TC0 += 1;
     }
     TIOS &= ~0x01;
} 


void openAD0 (void)        //opem AD0 
{

	ATD0CTL2 	= 0xE0;
	delayby50us(1);
	ATD0CTL3 	= 0x00; 
	ATD0CTL4 	= 0x65;   //0110 0101   16 bit conversion clock period
	
}


void sendLCD(char cmd, short mode)   // cmdLCD and putLCD here
{
     char temp;
     temp = cmd;          
     if(mode == 0){
        cmd &=0xF0;           
        LCD_DAT &= (~LCD_RS); /* select LCD instruction register */
        LCD_DAT |= LCD_E;     /* pull E signal to high */
     }else{
        
        LCD_DAT |= LCD_RS;    /* select LCD data register */
        LCD_DAT |= LCD_E;     /* pull E signal to high */
        cmd &= 0xF0;           /* clear the lower 4 bits */
     }
     cmd >>= 2;            /* shift to match LCD data pins */
     if(mode == 0){
      LCD_DAT = cmd | LCD_E;/* output upper 4 bits, E, and RS */
     }else{ 
     
      LCD_DAT = cmd|LCD_E_RS;/* output upper 4 bits, E, and RS */
     
     }
     asm("nop");           
     asm("nop");           
     asm("nop");
     LCD_DAT &= (~LCD_E);  
     cmd = temp & 0x0F;    
     LCD_DAT |= LCD_E;    
     cmd <<= 2;            /* shift to match LCD data pins */
     
     if(mode == 0){
      LCD_DAT = cmd | LCD_E;/* output upper 4 bits, E, and RS */
     }else{
      LCD_DAT = cmd|LCD_E_RS;/* output upper 4 bits, E, and RS */  
     }
     
     asm("nop");           /* dummy statements to lengthen E */
     asm("nop");           /*       "         */
     asm("nop");
     LCD_DAT &= (~LCD_E);  /* pull E clock to low */        
     delayby50us(2);//layby50us(1);       /* wait until the command is complete */
   
}

void openLCD(void)
{
     LCD_DIR = 0xFF;       /* configure LCD_DAT port for output */
     delayby50us(200);             
     sendLCD(0x28,0);        /* set 4-bit data, 2-line display, 5x7 font */
     sendLCD(0x0F,0);        /* turn on display, cursor, blinking */
     sendLCD(0x06,0);        /* move cursor right */
     sendLCD(0x01,0);        /* clear screen, move cursor to home */
     delayby50us(40);        /* wait until "clear display" command is complete */
}


void putsLCD (char *ptr)  //strings
{
     while (*ptr) 
     {
           sendLCD(*ptr,1);
           ptr++;
     }
}




int get_result(){
   
      int msb; 
      result = 0;
      result += ATD0DR0;
      result += ATD0DR1;
      result += ATD0DR2;
      result += ATD0DR3;
      result += ATD0DR4;
      result += ATD0DR5;
      result += ATD0DR6;
      result += ATD0DR7;   
      result = result >> 3;       
      
      msb = result >> 2;
      PORTB = msb;
      return result;  
      
}

void print_err(){
   
     sendLCD(0xC0,0);
     putsLCD(clear);
     sendLCD(0xC0,0);
     putsLCD("Position = ");
     putsLCD("err");
     delayby50us(50);      
     newline();
     putSCI("err");
   
   
}

void set_position(int pos, char *buff, char *lcd_message){

   
   
     if(pos >= -900 && pos <= 900){
      
       PWMDTY45 = (27705 - (pos*33/20));
       sendLCD(0xC0,0);
       putsLCD(clear);
       sendLCD(0xC0,0);
       putsLCD(lcd_message);
       putsLCD(&buff[0]);
       delayby50us(50); 
        
     }else{
      
      print_err();
      
     }

   
} 
  
void interrupt 25 handler()    
{
 
            
     if(bounce <= 0){
         
         
         push_counter++;    
         bounce = 10000;
         sound_counter = 0; 
         sound_case = 0;
         sound_flag = 0;
         temp_d = 0;
       
       
     } 

 
  PIFH  = 0x01;    
             
}


void interrupt 13 noise() 
{
    
    
  	 if(HiorLo_sound){
  	 	TC5 += HiCnt_sound;
  		HiorLo_sound = 0;
  	 }
  	 else{
  	 	TC5 += LoCnt_sound;
  		HiorLo_sound = 1;
  	 }	   

}    
   


void interrupt 15 timer() 
{       
      
        if(TFLG1 & 0x80)             
        {
      
            counter++;
            sound_counter++; 
            
            if( ((sound_counter % 20000) == 0) && sound_flag == 1) 
            {
                 
                 sound_case++;  
                                   
            } 
            if(bounce >= 0)
            {
              
              bounce--; 
            }
            
            //auto 
            if(push_counter == 3 && (counter % (20000/speed) == 0)) 
            {
                  
                  
                  if (direction == 0 && position_2 <= 26220){
                  
                          direction = 1;
                          
                  }else if( direction == 0 ){
                    
                          position_2 -= (165/10); 
                          
                  }else if(direction == 1 && position_2 >= 29190){
                    
                           direction = 0;    
                  }else{
                     
                          position_2 += (165/10);  
                  }
                  
                
              
                  PWMDTY45 = position_2;	         
  	         
  	        }
  	       
  	       
         TFLG1 = 0xFF;     
        }
                                 
     	 if(HiorLo_timer)
     	 {
      	 	TC7 += HiCnt_timer;
      		HiorLo_timer = 0;
        
    	 }
    	 else
    	 {
    	 	TC7 += LoCnt_timer;
    		HiorLo_timer = 1;
    	 }
   
}   

void main (void) 
{
  char *degree = "Degrees = ";
  char *temp_degree = "Temp = ";
  char *auto_mode = "DPS = "; 
  char *sci_message = "Enter Position = ";
  char *sci_auto = "Enter speed (1-9) = ";
  char *msg = "LCD is working!";
  char *lcd_message = "Position = ";  
   
  short push_temp; 

  int ones;
  int tens;
  int degrees;
  int postion =0; 
  direction = 0;
  speed = 0;
   
   

 
  PIFH    = 0x01; 
  DDRH    = 0xFE; 
  PIEH    = 0x01; 
  PERH    = 0x01; 
  PPSH    = 0xFE;
  HPRIO = 0xCC;
   
  
  PWMCLK = 0;	 
	PWMPOL = 0x00;     
	PWMCTL = 0x4C;  
  PWMPRCLK = 4;   
  PWMCAE   = 0;   /* select left alighed mode */
	PWMPER45 = 30000;  
	PWMDTY45 = 27750;	  // 0 deg, middle
	PWME |= 0x30;	

  push_counter = -1;
  push_temp = push_counter;
  bounce = 0;
	SetClk8();
	position_2 = 27750;	


  DDRP = 0xFF;
  DDRB    = 0xFF;                         
  DDRJ = 0x02; /* configure port J for output */
  PTJ = 0x02;
  
  TSCR1 = 0x90; 
  TSCR2 = 0x00; // presclaer 0f 1 
  
  TIOS  |= 0x80;  
  TIOS  &= ~0x20;
  TCTL1 = 0x04;  
  TIE   |= 0x80; 
  TIE   |= 0x20;
  
  HiCnt_timer = 1200;   // 50 us
  LoCnt_timer = 1200;
  HiorLo_timer =1;
  TC7 = TCNT + HiCnt_timer;
  set_null(buffer_2,7);
  set_null(buffer,8);
   buffer[0] = 0x20;
   buffer[1] = 0x20; /* space character */
   buffer[4] = 'd'; /* volt character */
   buffer[5] = 0;   /* null character */
   buffer[7] = 0;
   openLCD();
   openAD0();  
   sendLCD(0xC0,0);      
   putsLCD(msg);
   openSCI0();  
     
   asm(" cli ");  	    	
   while(1)
  	{
  	
  	  if(push_temp !=  push_counter){     //updates between cases 
         
       
          if(push_counter > 4)
      	  {   
             push_counter = 0;
          }
      
        // buffer_2[0] = 0;
         set_null(buffer_2,7);
         set_null(buffer,8);
       
       
          buffer[0] = 0x20;
          buffer[1] = 0x20; /* space character */
          buffer[4] = 'd'; /* volt character */
          buffer[5] = 0;   /* null character */
         TIOS &= ~0x20;
  	     newline();
  	     putSCI("Mode: ");//to SCI
  	     putSCI(mode[push_counter]); //to SCI 
         newline(); 
  	     if(push_counter == 3 || push_counter == 4){
               putSCI("Enter ! to escape.");//to SCI 
          }
  	     sendLCD(0x02,0);  
         putsLCD(clear);
         sendLCD(0xC0,0);
         putsLCD(clear);
         sendLCD(0x02,0);   
         putsLCD(mode[push_counter]);
         sendLCD(0xC0,0); 
         push_temp = push_counter; 
         sendLCD(0x10,0);
           
  	  }
  	  if(push_counter == 0) //420 mode   pot
  	  {   
  	    
  	    TIOS |= 0x20;
  	    ATD0CTL5 = 0x87;                /* convert AN7, result right justified */
  	    
        while(!(ATD0STAT0 & 0x80));      /* wait for conversion to complete */
         
           result = get_result();               
      
           degrees = 0;
           degrees = (((result * 22)/125)) - 90;
            
            if(degrees < 0){
            
              degrees = degrees * -1;
              buffer[1] = 0x2D;
               
            } else {
            
               buffer[1] = 0x20; 
               
            }
            buffer[2] = 0x30 + (degrees / 10); 
            buffer[3] = 0x30 + (degrees % 10); 
        
            
        
            HiCnt_sound  = (((1023 - result) * 25)/4)+ 9600;
            LoCnt_sound  = (((1023 - result) * 25)/4)+ 9600 ;
            
            PWMDTY45 = 29190 - (result *(29/10)); 
            
        
             clearline(); 
             carriage_return();
             putSCI(degree);                   
             putSCI(&buffer[0]); 
        
                        
            sendLCD(0xC0,0);
            putsLCD(degree);
            putsLCD(&buffer[0]);
            delayby50us(50);

  	  } 
  	  else if(push_counter == 1) 
  	  {            

  	      ATD0CTL5 = 0x85;
  	      PWMDTY45 = 27750;	 
  	      while(!(ATD0STAT0 & 0x80));   
  	      
          result = get_result();               
      
          degrees = 0;
          degrees = (((result * 20)/41));
          
          if(temp_d == 0){ 
            temp_d = degrees;
             sound_flag = 0;
           
            }
          
          
          if(degrees >= (temp_d + 5)){
          
              TIOS |= 0x20;     //on
              sound_flag = 1;
             
          }else{
          
             TIOS &= ~0x20;     //on
             sound_flag = 0;
              
          }
         
         if(sound_flag == 1){
           
          HiCnt_sound = sound[sound_case%3];
          LoCnt_sound = sound[sound_case%3];
          
         }
             
            
          
          buffer[0] = 0x30 + (degrees / 10); 
          buffer[1] = 0x30 + (degrees % 10);   
         
          buffer[2] = 0x43;
          
          buffer[3] = 0x20; 
          buffer[4] = 0x30 + (((degrees * 9)/5) + 32) /10;
          buffer[5] = 0x30  + (((degrees * 9)/5) + 32) % 10;
          buffer[6] = 0x46;
         
        
           clearline(); 
           carriage_return();
           putSCI(temp_degree); 
           putSCI(&buffer[0]); 
          
        
      	    sendLCD(0xC0,0);
      	    putsLCD(temp_degree);
            putsLCD(&buffer[0]);
            delayby50us(50);
  	   
   
  	  }
  	  else if(push_counter == 2){  //light
  	      TIOS |= 0x20;
  	     // openAD0();  
  	      ATD0CTL5 = 0x84;
  	      while(!(ATD0STAT0 & 0x80));      /* wait for conversion to complete */
  	      
            result = get_result();               
        
             degrees = 0;
             degrees = (((result * 22)/125)) - 90;
              
              if(degrees < 0){
              
                degrees = degrees * -1;
                buffer[1] = 0x2D;
                 
              } else {
              
                 buffer[1] = 0x20; 
                 
              }
              buffer[2] = 0x30 + (degrees / 10); 
              buffer[3] = 0x30 + (degrees % 10); 
          
              
        
          PWMDTY45 = (result *(29/10)) + 26220;  
          
  	      HiCnt_sound  = (((1023 - result) * 25)/4)+ 9600;
          LoCnt_sound  = (((1023 - result) * 25)/4)+ 9600 ;
          
          
          
         /* update SCI */ 
           clearline(); 
           carriage_return();
           putSCI(degree); //to SCI 
           putSCI(&buffer[0]); //to SCI 
          /* update SCI */ 
          sendLCD(0xC0,0);
          putsLCD(degree);
          putsLCD(&buffer[0]);
          delayby50us(50);
  	
  	  } 
  	  
  	  else if(push_counter == 3)  //auto 
  	  {      
  	                     

  	     set_null(buffer_2,7);
  	     
  	     
  	     
  	     
  	     if(buffer_2[0] != 0x21)
  	     {  //"!" <- break out 
  	        
  	        newline();
            putSCI(sci_auto); //to SCI 
            set_null(buffer_2,7);
  	        getstr(&buffer_2[0]);//get string from user.  //degrees -> PWM  //get input 
  	       
  	    
  	  
  	        if(buffer_2[0] == 0x21)
  	        {
  	        
                newline();
                putSCI("END press sw5"); //to SCI
                delayby50us(50);
                     
  	        } else if(is_digit(buffer_2[0]) && length(buffer_2,7) == 1 && (buffer_2[0] - '0') > 0)
  	        {
  	               
  	               speed = buffer_2[0] - '0'; 
  	               buffer[0]  = 0x30 + speed;           
            	     sendLCD(0xC0,0);
            	     putsLCD(clear);
            	     sendLCD(0xC0,0);
                   putsLCD(auto_mode );
                   putsLCD(&buffer[0]);
                   
                   
  	        }else{
    	        
    	         print_err(); 
  	           
  	        }
                    
      
	  
    	    }
  	  
  	 
  	  }else if( push_counter == 4)   //SCI 
  	  { 
     
         int neg_off = 0;
         
  	     if(buffer_2[0] != 0x21){  //"!" <- break out 
  	        
  	        newline();
            putSCI(sci_message); //to SCI 
  	        position = 0;
            set_null(buffer_2,7);
  	        getstr(&buffer_2[0]);//get string from user.  //degrees -> PWM  //get input 
  	       
  	        if(buffer_2[0] == 0x2D)
  	        {
  	          
  	           neg_off = 1;
  	        
  	        }else
  	        {
  	          
               neg_off = 0;
  	        }
            
  	  
  	        if(buffer_2[0] == 0x21){
  	        
                newline();
                putSCI("END press sw5"); //to SCI
                delayby50us(50);
                     
  	        }else if(length(buffer_2,7) == 4+neg_off && buffer_2[2+neg_off] == 0x2E && is_digit(buffer_2[1+neg_off]) && is_digit(buffer_2[1+neg_off]) && is_digit(buffer_2[3+neg_off]))
              {
                         
                   position += ((int)buffer_2[0+neg_off] - '0')* 100;
                   position += ((int)buffer_2[1+neg_off] - '0') * 10 ;
                   position += ((int)buffer_2[3+neg_off] - '0');
                   set_position(position,buffer_2,lcd_message);
                   
                 
                   
               } else if(length(buffer_2,7) == 3+neg_off && buffer_2[1+neg_off] == 0x2E && is_digit(buffer_2[0+neg_off]) && is_digit(buffer_2[2+neg_off]))
               {
                   
                   position += ((int)buffer_2[0+neg_off] - '0') * 10;
                   position += ((int)buffer_2[2+neg_off] - '0');
                   set_position(position,buffer_2,lcd_message);
                   
                 
                 
               } else if(length(buffer_2,7) == 2+neg_off && is_digit(buffer_2[0+neg_off]) && is_digit(buffer_2[1+neg_off]))
               {
                   position += ((int)buffer_2[0+neg_off] - '0') * 100;
                   position += ((int)buffer_2[1+neg_off] - '0') * 10;
                   set_position(position,buffer_2,lcd_message);
                 
                 
                 
               } else if(length(buffer_2,7) == 1+neg_off && is_digit(buffer_2[0+neg_off]))
               {
               
                   position += ((int)buffer_2[0+neg_off] - '0') * 10;
                   set_position(position,buffer_2,lcd_message);
               
                   
                    
               }else
               {
               
               
                  print_err(); 
                       
                  
               }
           }
  	  }
  

     
   }       
}