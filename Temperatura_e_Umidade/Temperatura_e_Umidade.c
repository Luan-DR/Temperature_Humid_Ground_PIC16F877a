//#include <Temperatura_e_Umidade.h>
#include <16F877A.h>
#device ADC=8

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(crystal=20MHz)

//Função da Temperatura obtida no site https://simple-circuit.com/pic16f877a-ds18b20-temperature-sensor/
//Conexão LCD igual ao PicsimLab, mas como não usei os pinos D0, D1, D2, D3 eles não estão conectados, e o pino V0 está conectado a um potenciomentro.
//A biblioteca LCD é a mesma trabalhada em aula.

#ifndef lcd_enable                                                             
   #define lcd_enable     pin_e1   
   #define lcd_rs         pin_e2   
   #define lcd_d4         pin_d4      
   #define lcd_d5         pin_d5      
   #define lcd_d6         pin_d6     
   #define lcd_d7         pin_d7      
#endif

#include <mod_lcd.c>

#define DS18B20_PIN PIN_B1 //Pino do sensor de temperatura.
signed int16 raw_temp;
float temp;
int16 umid_a, umid_d;
 
int1 ds18b20_start(){                              // Usado para saber se o sensor DS18B20 está corretamente conectado ao circuito, retorna TRUE (1) se OK e FALSE (0) se errar.
  output_low(DS18B20_PIN);                         // Envie pulso de reset para o sensor DS18B20.
  output_drive(DS18B20_PIN);                       // Configure o pino DS18B20_PIN como saída.
  delay_us(500);                                  
  output_float(DS18B20_PIN);                       // Configure o pino DS18B20_PIN como entrada.
  delay_us(100);                                   
  if (!input(DS18B20_PIN)) {
    delay_us(400);                                
    return TRUE;                                   // Sensor DS18B20 está presente.
  }
  return FALSE;
}
 
void ds18b20_write_bit(int1 value){                // Grava (envia) 1 bit para o sensor DS18B20, o bit é 'valor' que pode ser 1 ou 0.
  output_low(DS18B20_PIN);
  output_drive(DS18B20_PIN);                       // Configure o pino DS18B20_PIN como saída.
  delay_us(2);                              
  output_bit(DS18B20_PIN, value);
  delay_us(80);                                  
  output_float(DS18B20_PIN);                       // Configure o pino DS18B20_PIN como entrada.
  delay_us(2);                                    
}
 
void ds18b20_write_byte(int8 value){               // Grava 1 byte (8 bits) para o sensor DS18B20, esta função é baseada na função anterior. Esta função grava LSB (Bit Menos Significante) primeiro.
  int8 i;
  for(i = 0; i < 8; i++)
    ds18b20_write_bit(bit_test(value, i));
}
 
int1 ds18b20_read_bit(void) {                      // Lê 1 bit do sensor DS18B20, retorna o valor de leitura (1 ou 0).
  int1 value;
  output_low(DS18B20_PIN);
  output_drive(DS18B20_PIN);                       // Configure o pino DS18B20_PIN como saída.
  delay_us(2);
  output_float(DS18B20_PIN);                       // Configure o pino DS18B20_PIN como entrada.
  delay_us(5);                                    
  value = input(DS18B20_PIN);
  delay_us(100);                               
  return value;
}
 
int8 ds18b20_read_byte(void) {                     // Lê 1 byte (8 bits) do sensor DS18B20, esta função é baseada na função anterior. Esta função lê LSB primeiro.
  int8 i, value = 0;
  for(i = 0; i  < 8; i++)
    shift_right(&value, 1, ds18b20_read_bit());
  return value;
}
 
int1 ds18b20_read(int16 *raw_temp_value) {  // Lê os dados brutos de temperatura que tem 16 bits de comprimento (dois registros de 8 bits), os dados são armazenados na variável raw_temp_value, retorna TRUE se OK e FALSE se erro.
  if (!ds18b20_start())                     // Enviar pulso inicial.
    return FALSE;
  ds18b20_write_byte(0xCC);                 // Enviar comando pular ROM.
  ds18b20_write_byte(0x44);                 // Enviar comando de conversão inicial.
  while(ds18b20_read_byte() == 0);          // Aguarde a conclusão da conversão.
  if (!ds18b20_start())                     // Enviar pulso inicial.
    return FALSE;                           // Retorne 0 se houver erro.
  ds18b20_write_byte(0xCC);                 // Enviar comando pular ROM.
  ds18b20_write_byte(0xBE);                 // Enviar comando de leitura.
  *raw_temp_value = ds18b20_read_byte();    // Ler byte LSB de temperatura e armazená-lo em raw_temp_value LSB byte.
  *raw_temp_value |= (int16)(ds18b20_read_byte()) << 8; //Ler byte MSB de temperatura e armazená-lo em raw_temp_value byte MSB.
  return TRUE; // OK --> return 1
}
 
void main() {
      setup_adc_ports(AN0); //Pino do sensor de umidade.
      setup_adc(ADC_CLOCK_INTERNAL);

      lcd_ini();
      delay_ms(50);
    
  while(TRUE) {
    if(ds18b20_read(&raw_temp)) {
      temp = (float)raw_temp / 16; //Convert temperature raw value into degree Celsius (temp in °C = raw/16).
      printf(lcd_escreve,"\n\fTemp %f", temp); //Escreve "Temp" e o valor da temperatura.
      lcd_escreve(223); //Escreve o simbolo "°".
      lcd_escreve("C"); //Escreve "C".
                                 
    }
    else {
       printf(lcd_escreve,"\n\fSensor Temp ERRO"); //Caso não encontre o sensor de temperatura, envia uma mensagem de ERRO.
    }
    delay_us(10);
    umid_a = read_adc(); //Leitura do sensor de umidade.
    umid_d = 100-((umid_a*100)/255); //Transforma em porcentagem, e deixa em nivel maximo 0% e nivel minimo 100%.
    printf(lcd_escreve,"\rUmid %ld", umid_d); //Escreve "Umid" e o valor da Umidade.
    lcd_escreve(37); //Escreve o simbolo "%".
    lcd_escreve("                   "); //Dá um espaçamento para o blink do display não mostrar na tela.
    delay_ms(20);
  }
}
