#define ERROR 0
#define DEBUG 2
#define HIPFT_PORT  7771
#define HIPFT_PATH "/tmp/test"
#define HIPFT_BLOCK_SIZE 4096
// choose frequency of roughly every block but move around a fair bit
#define ERRFREQ (5*(HIPFT_BLOCK_SIZE-13)) //insert error every ERRFREQ
