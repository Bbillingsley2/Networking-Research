// Algorithm
//
// while 1
//    start with empty bytes
//    read until special char or cant read anymore
//    if ( cant read anymore )
//       BAD_BYTES_HANDLER with LOG_METHOD
//       return NULL
//    if ( verify COBS is bad )
//       BAD_BYTES_HANDLER with LOG_METHOD
//       continue
//    decode COBS bytes
//    form message with bytes
//    if ( message does not use all bytes or theres not enough )
//       BAD_BYTES_HANDLER with LOG_METHOD
//       continue
//    if ( verify message is bad )
//       BAD_BYTES_HANDLER with LOG_METHOD
//       continue
//    return valid message
//
// possibly move the semphore to the read message instead of on the bits

void bad_bytes_handler(void){
  // decode the error
  // log method
  return;
}

// sketch of revised recv_msg
msg_t *recv_msg(int sock) {
  uchar8* read_bytes, decoded_bytes;
  uint64 read_length, decoded_length, readin;
  uchar8 c; // temp for one character
  msg_t* res;
  // TODO: add cleanups and frees
  while(1){
    read_length = 0;
  	// read until special char or cant anymore
    // TODO: use select here
  	while((readin = readn(sock,&c,1)) > 0){
      read_length += readin;
      // add c to read_bytes
  	  if(c == SPECIAL_CHAR){
        break;
      }
  	}
    // if we couldnt read anymore
    if(readin <= 0){
      bad_bytes_handler();
      return NULL;
  	}
    // if COBS encoding markers are corrupted
    if(cobs_verify(read_bytes, read_length) != true){
      bad_bytes_handler();
      continue;
    }
    // decode COBS bytes
    decoded_length = cobs_decoded_length_from_encoded(read_bytes)
    decoded_bytes = malloc(decoded_length);
    cobs_decode(read_bytes, read_length, decoded_bytes);
    // form message with bytes
    res = msg_from_bytes(decoded_bytes, decoded_length);
    // if there are more bytes after than the message takes
    if(res == NULL){
      bad_bytes_handler();
      continue;
    }
    // if message integrity is bad
    if(verify_msg(res) == false){
      bad_bytes_handler();
      continue;
    }
    // return completely valid message
    return res;
  }
  // only in case of an error
  return NULL;
}
