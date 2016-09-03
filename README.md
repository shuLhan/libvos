# libvos

libvos is a C++ library. First implementation was intended for reading and
writing Delimited Separated Value (DSV) data only, but then evolved and have
more capabilities.

libvos contain several modules that can be independent or dependable through
class inheritance and include pragma.

Current modules and features,

* Buffer module, very easy and simple module for processing sequence
  byte of data. In example: string or network packet.

* File module, module to talk to local file system, reading and writing
  file.

* Dir module, module to talk to local file system, listing and caching
  directory content.

* Reading and writing Delimited Separated Value (DSV) data.

* Reading and writing INI file format.

* Socket module, talk to network using UDP or TCP socket.

* FTP module, very easy and simple module to talk to FTP server.

* FTP server, module for serving file system or any hierarchy of data to
  networks using FTP.

* OCI module, Minimalist and simple module for sending a query to Oracle
  Database server and receiving query data (not fully supported).


For more information see libvos Documentation [1] .

Get the latest libvos source code here [2] .

## License

Copyright 2009-2016, M. Shulhan (ms@kilabit.info).
All rights reserved.

 	---	---	---	---	---	---	---

	TT  TT   II  BB          AAAA     LLLLLL  II  KKKKKKKK
	TT TT    II  BB         AA  AA   LL   LL  II     KK
	TTTT     II  BB        AA    AA   LL  LL  II     KK
	TT TT    II  BB        AAAAAAAA   LLLLLL  II     KK
	TT  TT   II  BB        AA    AA  LL   LL  II     KK
	TT   TT  II  BBBBBBBB  AA    AA   LLLLLL  II     KK

Website: http://kilabit.info
Contact: ms@kilabit.info

--

[1] http://kilabit.info/projects/libvos/doc/index.html

[2] http://github.com/shuLhan/libvos
