# hipFT | secure-edc: Is the typical 16-bit checksum enough?

The most common internet protocols (UDP + TCP) use a 16-bit checksum (one's complement) in order to detect errors found within the data portion of a packet. A packet in both protocols is 65,535 bytes. However, both use their own headers and trailers along with typical IP information. Regardless of their differences, it can be said that this 16-bit checksum is expected to cover up to 64-65k worth of bytes.

Currently, we are seeing errors that get past this checksum in both protocols seen in long connections and big downloads. TCP-IP, however, is the most susceptible as most applications do not add any more error detection on top of this protocol. This makes sense since it is deemed as 'reliable'.

Our mission is to see how much errors currently get through in a real world scenario of transfer large files between two machines. This will be used to determine how much more
error detection we need to add to these existing protocols to see better connectivity and efficiency in long connections and large downloads.

## Running

To run the bare-bones sftp we are using to detect errors that are getting past TCP-IP, do the following.

* Clone our repo. Move into the main directory.
```
git clone https://github.com/hipft/basic_server_client.git
cd basic_server_client
```

* Make all executables
```
make
```

* Test all modules
```
./build/test
```

* Running the Server
```
./build/server PORT
./build/server 7777
```

* Running the Client
```
./build/client IP PORT FILE #DOWNLOADS
./build/client localhost 7777 sample.txt 3
```

* Clean after running
```
make clean
cd ..
rm -rf ./basic_server_client
```

## Notes

Ensure syslogs will be present:
* service syslog {stop/status/restart/start}
* logs are in /var/log/syslog
