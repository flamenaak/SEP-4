/*
 * display.c
 *
 * Created: 31-05-2017 20:23:47
 *  Author: 224469
 */ 

#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t add_parity_bits(unsigned char data){
char data_array[8] = {};
for(uint8_t i = 7; i >= 0; i--){
if(data & (1 << i)) data_array[7-i] = 1; else data_array[7-i] = 0;
}
char parity_bits[4] = {};
parity_bits[0] = (data_array[0] + data_array[1] + data_array[3] + data_array[4] + data_array[6]) % 2;
parity_bits[1] = (data_array[0] + data_array[2] + data_array[3] + data_array[5] + data_array[6]) % 2;
parity_bits[2] = (data_array[1] + data_array[3] + data_array[6] + data_array[7] + data_array[8]) % 2;
parity_bits[3] = (data_array[5] + data_array[6] + data_array[7]) % 2;

uint8_t trailer = 0;
for(uint8_t i = 4; i >= 0; i--){
if(parity_bits[4-i]) trailer += 1<<i;
}
data << 3;
return data + trailer;
}

uint8_t check_parity_bits(unsigned char data){

uint8_t index_array[12] = {3,2,11,1,10,9,8,0,7,6,5,4};
char data_array[12] = {};
for(uint8_t i = 11; i >= 0; i--){
if(data & (1 << i)) data_array[11-i] = 1; else data_array[11-i] = 0;
}
char parity_bits[4] = {};
parity_bits[0] = (data_array[3] + data_array[11] + data_array[10] + data_array[8] + data_array[7] + data_array[5]) % 2;
parity_bits[1] = (data_array[2] + data_array[11] + data_array[10] + data_array[9] + data_array[6] + data_array[5]) % 2;
parity_bits[2] = (data_array[1] + data_array[10] + data_array[9] + data_array[4] + data_array[5] + data_array[4]) % 2;
parity_bits[3] = (data_array[0] + data_array[7] + data_array[6] + data_array[5]) % 2;

if(parity_bits[0]  ||  parity_bits[1]  || parity_bits[2]  ||  parity_bits[3]); //ok
else {
uint8_t damaged_bit_pos = 0;
if(parity_bits[0])  damaged_bit_pos += 1;
if(parity_bits[1])  damaged_bit_pos += 2;
if(parity_bits[2])  damaged_bit_pos += 4;
if(parity_bits[3])  damaged_bit_pos += 8;

if(data_array[index_array[damaged_bit_pos-1]]) data_array[index_array[damaged_bit_pos-1]] = 0; else data_array[index_array[damaged_bit_pos-1]] = 1;
}

uint8_t payload = 0;
for(uint8_t i = 4; i >= 0; i--){
if(data_array[4-i]) payload += 1<<i;
}
return payload;

}

