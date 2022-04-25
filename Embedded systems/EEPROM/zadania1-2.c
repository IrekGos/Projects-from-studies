#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "i2c.h"

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

FILE uart_file;

#define ADDRESS_READ 0b10100001
#define ADDRESS_WRITE 0b10100000

uint8_t read_one_byte(uint16_t address)
{
  i2cStart();
  i2cSend(ADDRESS_WRITE);
  i2cSend(address & 0xff);
  i2cStart();
  i2cSend(ADDRESS_READ);
  uint8_t data = i2cReadNoAck();
  i2cStop();
  return data;
}

void write_one_byte(uint16_t address, uint8_t value)
{
  i2cStart();
  i2cSend(ADDRESS_WRITE);
  i2cSend(address & 0xff);
  i2cSend(value);
  i2cStop();
}

void read_n_bytes(uint16_t address, uint8_t length, uint8_t *tab)
{
  i2cStart();
  i2cSend(ADDRESS_WRITE);
  i2cSend(address & 0xff);
  i2cStart();
  i2cSend(ADDRESS_READ);
  for (uint8_t i = 0; i < length - 1; i++)
    tab[i] = i2cReadAck();
  tab[length - 1] = i2cReadNoAck();
  i2cStop();
}

void write_n_bytes(uint16_t address, uint8_t length, uint8_t *value)
{
  i2cStart();
  i2cSend(ADDRESS_WRITE);
  i2cSend(address & 0xff);
  for (uint8_t i = 0; i < length; i++)
    i2cSend(value[i]);
  i2cStop();
}

uint16_t to_two_complement(uint16_t sum)
{
  return (~sum + 1) & 0x00ff;
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj I2C
  i2cInit();
  // program testowy
  uint16_t address = 0;
  uint8_t value, length = 0;
  char command[10];
  printf("********************** Interface description **********************\r\n");
  printf("* read addr – read one byte from addr                             *\r\n");
  printf("* write addr value – write value to addrr                         *\r\n");
  printf("* readn addr length – read length bytes from addr in I8HEX format *\r\n");
  printf("* writen – write rows given in I8HEX format                       *\r\n\n");
  while (1)
  {
    scanf("%s", command);
    if (!strcmp(command, "read"))
    {
      scanf("%x", &address);
      value = read_one_byte(address);
      printf("\r\nAdress: %#.3x value: %#.2x\r\n", address, value);
    }
    else if (!strcmp(command, "write"))
    {
      scanf("%x %hhx", &address, &value);
      write_one_byte(address, value);
      printf("\r\nAdress: %#.3x value: %#.2x\r\n", address, value);
    }
    else if (!strcmp(command, "readn"))
    {
      scanf("%x %" SCNd8, &address, &length);
      uint8_t tab[length];
      read_n_bytes(address, length, tab);
      uint16_t sum = 0;
      for (int i = 0; i < length; i++)
        sum += tab[i];
      sum &= 0x00ff;
      sum = to_two_complement(sum);
      printf("\r\n:%.2X%.4X00", length, address);
      for (int i = 0; i < length; i++)
        printf("%.2X", tab[i]);
      printf("%.2X\r\n", sum);
    }
    else if (!strcmp(command, "writen"))
    {
      while (1)
      {
        printf("\r\n");
        char i8hex[80];
        scanf("%s", i8hex);
        uint8_t record_type;
        sscanf(i8hex, ":%2hhX%4X%2hhX", &length, &address, &record_type);
        if (record_type == 1)
        {
          printf("\r\n'End Of File' string detected (Record type = 01)\r\n");
          break;
        }
        uint8_t tab[length], sum = 0;
        for (int i = 0; i < length; i++)
        {
          sscanf(i8hex + 9 + 2 * i, "%2hhX", &tab[i]);
          sum += tab[i];
        }
        int8_t checksum;
        sscanf(i8hex + 9 + 2 * length, "%2hhX", &checksum);
        if (checksum != to_two_complement(sum))
        {
          printf("\r\n\nChecksum is incorrect!!!\r\n");
          printf("Correct checksum equals: %x\r\n", to_two_complement(sum));
        }
        else
          write_n_bytes(address, length, tab);
      }
    }
    else
      printf("\r\nUnknown command!\r\n");
    printf("-------------------------\r\n");
  }
}