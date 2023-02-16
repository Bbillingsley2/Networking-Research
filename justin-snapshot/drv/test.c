#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../hdr/stdtypes.h"
#include "../hdr/cobs.h"
#include "../hdr/byte_help.h"
#include "../hdr/byte_order.h"
#include "../hdr/file_handle.h"
#include "../hdr/parity.h"
#include "../hdr/message.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/conn_chan.h"
#include "../hdr/section.h"

void cobs_testing(void){
  printf("*******************\n");
  printf("COBS TESTING\n");
  printf("*******************\n\n");

  uint64 raw_size = 255;
  uchar8* raw_data = malloc(raw_size);
  for(uint32 i=0; i<raw_size; i++){ raw_data[i] = 'a'; }
  uint64 cobs_size = cobs_encoded_length_from_decoded(raw_data, raw_size);
  uchar8* cobs_data = malloc(cobs_size);

  printf("Special Character for COBS: %02x\n", (uint32)SPECIAL_CHAR);
  printf("RAW Size: %lu\n", raw_size);
  printf("COBS Size: %lu\n", cobs_size);

  printf("RAW: "); print_hex(raw_data, raw_size);
  cobs_encode(raw_data, raw_size, cobs_data);
  printf("COBS: "); print_hex(cobs_data, cobs_size);

  printf("COBS Length from COBS: %lu\n", cobs_encoded_length_from_encoded(cobs_data));
  printf("RAW Length from COBS: %lu\n", cobs_decoded_length_from_encoded(cobs_data));

  free(raw_data);
  raw_data = malloc(raw_size);
  if(cobs_verify(cobs_data, cobs_size) != true){
      printf("Invalid COBS\n");
  }else{
      printf("Valid COBS\n");
  }
  cobs_decode(cobs_data, cobs_size, raw_data);

  printf("Decoded COBS (RAW): "); print_hex(raw_data, raw_size);
  printf("COBS Length from COBS: %lu\n", cobs_encoded_length_from_encoded(cobs_data));
  printf("RAW Length from COBS: %lu\n", cobs_decoded_length_from_encoded(cobs_data));

  free(raw_data);
  free(cobs_data);
}

void byte_testing(void){
  printf("\n*******************\n");
  printf("RANDOM INT / BYTE TESTING\n");
  printf("*******************\n\n");

  uint64 num = generate_random_uint(100, UINT64_MAX-1);
  uint64 i = hton64(num);
  uchar8* intbytes = malloc(sizeof(i));
  memcpy(intbytes, &i, sizeof(i));
  printf("Int (%lu) Hex: ", num);
  print_hex(intbytes, sizeof(i));
  uint64 temp;
  memcpy(&temp, intbytes, sizeof(temp));
  temp = ntoh64(temp);
  printf("int from bytes: %lu\n", temp);
  free(intbytes);

  uchar8* b1 = malloc(2);
  b1[0] = 'a';
  b1[1] = 'b';
  uchar8* b2 = malloc(2);
  b2[0] = 'c';
  b2[1] = 'd';
  uchar8* b3 = combine_voids(b1, 2, b2, 2);
  printf("Byte Combine (a,b)+(c,d): ");
  print_hex(b3, 4);
  free(b1);
  free(b2);
  free(b3);
}

void parity_testing(void){
  printf("\n*******************\n");
  printf("PARITY TESTING\n");
  printf("*******************\n\n");

  uint64 size = 32;
  uchar8* data = malloc(sizeof(uchar8) * size);
  for(uint8 i=0; i<size; i++){ data[i] = 'a'; }

  uint64* vpart = malloc(compute_vparity_length());
  compute_vparity(data, size, vpart);
  for(uint8 i=0; i<4; i++){
      printf("Vdata: %lu\n", vpart[i]);
  }

  if(check_vparity(data, size, vpart) == true){
      printf("VALID Vdata\n");
  }else{
      printf("CORRUPTED Vdata\n");
  }

  uchar8* hpart = malloc(compute_hparity_length(size));
  compute_hparity(data, size, hpart);

  if(check_hparity(data, size, hpart) == true){
      printf("VALID Hdata\n");
  }else{
      printf("CORRUPTED Hdata\n");
  }
  printf("Vdata Size: %lu, Hdata Size: %lu\n", compute_vparity_length(), compute_hparity_length(size));
  free(data);
  free(vpart);
  free(hpart);

  uint64 temp = 561;
  printf("Buff Length of %lu is %lu.\n", temp, compute_buff_length(temp, BUFFER_CHUNK_SIZE));
  printf("uint64 to uint32: %u\n", (uint32)temp);
}

void message_testing(void){
  printf("\n*******************\n");
  printf("MESSAGE TESTING\n");
  printf("*******************\n\n");

  uint32 sizetotal = 10;
  uchar8* bytes = malloc(sizeof(uchar8) * sizetotal);
  for(uint8 i=0; i<sizetotal; i++){ bytes[i] = 'a'; }
  msg_t* src = form_msg(100, 200, DATA, SYN, 0, bytes, sizetotal);
  free(bytes);
  print_msg(src, true, true, true);
  printf("\n\n\n");

  bytes = NULL;
  uint64 size = 0;
  msg_to_bytes(src, &bytes, &size);
  printf("Message Bytes: (%lu) ", size);
  print_hex(bytes, size);

  uint64 encoded_size = cobs_encoded_length_from_decoded(bytes, size);
  uchar8* encoded_bytes = malloc(encoded_size);

  cobs_encode(bytes, size, encoded_bytes);

  printf("\nSent Bytes including COBS: (%lu) ", encoded_size);
  print_hex(encoded_bytes, encoded_size);
  printf("\n\n\n");

  free(bytes);
  bytes = malloc(size);
  cobs_decode(encoded_bytes, encoded_size, bytes);


  msg_t* dst = msg_from_bytes(bytes);
//  msg_t* dst = msg_from_bytes(bytes, size);
  print_msg(dst, true, true, true);
  printf("\nValid Message = %s\n", (verify_msg(dst)) ? "VALID" : "CORRUPTED");

  free(bytes);
  free(encoded_bytes);
  free_msg(src);
  free_msg(dst);
}

void connchan_testing(void){
  printf("\n*******************\n");
  printf("CONN_CHAN TESTING\n");
  printf("*******************\n\n");

  connection_t** conns = NULL;
  uint64 totalConnections = 0;
  uint64 totalChannels = 0;
  new_connection(&conns, &totalConnections);
  new_connection(&conns, &totalConnections);
  new_connection(&conns, &totalConnections);
  new_channel(&conns, &totalChannels, 0);
  new_channel(&conns, &totalChannels, 1);
  new_channel(&conns, &totalChannels, 1);
  printf("total connects: %lu | total channels: %lu\n", totalConnections, totalChannels);
  print_all_connections(&conns, &totalConnections);
  printf("\n\n");

  printf("ending things...\n");
  end_all_connections(&conns, &totalConnections, &totalChannels);

  printf("total connects: %lu | total channels: %lu\n", totalConnections, totalChannels);
  print_all_connections(&conns, &totalConnections);
}

void section_testing(void){
  printf("\n*******************\n");
  printf("SECTION TESTING\n");
  printf("*******************\n\n");

  section_t* sec = create_section();
  add_to_section(sec, 5);
  add_to_section(sec, 6);
  add_to_section(sec, 7);
  add_to_section(sec, 8);
  print_section(sec); printf("\n");

  add_to_section(sec, 1);
  print_section(sec); printf("\n");

  delete_from_section(sec, 6);
  print_section(sec); printf("\n");

  add_to_section(sec, 6);
  add_to_section(sec, 2);
  add_to_section(sec, 3);
  add_to_section(sec, 4);
  print_section(sec); printf("\n");

  free_section(sec);
}

void file_testing(void){
  printf("\n*******************\n");
  printf("FILE_HANDLE TESTING\n");
  printf("*******************\n\n");

  uchar8* directory_str = (uchar8*)"./files/input";
  dir_handle_t* dhandle = open_dir_handle(strlen((char8*)directory_str), directory_str);
  print_dir_handle(dhandle);
  close_dir_handle(dhandle);
}

int main(void){
  cobs_testing();
  byte_testing();
  parity_testing();
  message_testing();
  connchan_testing();
  section_testing();
  file_testing();

  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
}
