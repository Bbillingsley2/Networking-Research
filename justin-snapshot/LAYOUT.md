# Layout of this Project

The layout is structured from bottom-up, meaning the first files you see here are the lowest-level of this project. Everything has both a header (hdr) file and a source (src) file unless specified or is a driver.

# Bare-Bones
* 'stdtypes': abbreviation and standardizes variable sizes, provides proper renaming

# Low-level Modules
* 'byte_help': allocation prints, copies, and general calculations having a number of bytes
* 'socket_help': ensures a set amount of bytes are read/write, thread-safe
* 'byte_order': converting network byte order to/from host byte order

# Modules
* 'cobs': a simple module for decoding/verifying/encoding cobs
* 'parity': a simple module for getting/checking/analyzing parity
* 'message': the message structure, a unit to be sent/receive
    - 'socket_msg_help': functions to help sent and receive messages
* 'conn_chan': handling and storing connections vs channels
* 'file_handle': helps with managing file pointers and accessing data correctly
* 'section': provides a way to see what I am "missing" in a section of data

# Driver Helpers
* 'server_help': a module that helps the driver server be more simplistic and structured

# Drivers
* 'test': a simply program to test all the modules above
* 'client': interacts with the server to fetch files
* 'server': provides requested files to the client