# uploadwpa
This module will upload a wpa handshake from a single capture file to an online hash cracker site!

BTC: 1GEKnNuVkMeMWiMcTQf8io2GqgCusQcK1E
<------------------------------------------------------------------------------------------------->
uploadwpa 1.0 ~ AlfAlfa
This module will upload a wpa handshake from a single capture file to www.onlinehashcrack.com
or you can give it up to 10 hashes seperated by spaces and of any hashes the site supports
Usage:
{Send WPA Handshake:}
uploadwpa -e youremail@yourdomain.com -c myaccesspoint.cap
uploadwpa -e email@yourdomain.com -c ~/captures/myaccesspoint.hccap -u "A Custom User Agent"
{Send up to 10 hashes at once of hashes supported by the site:}
uploadwpa -e youremail@yourdomain.com -a hash1 hash2 hash3 hash4 hash5 hash6 hash7 hash8 hash9 hash10
{Send both sequentially:}
uploadwpa -e example@example.com -a hash1 etc etc -c /path/to/capture.cap

1. Compile uploadwpa cli for linux:
	g++ -std=c++0x HTTPClient.cpp uploadwpa.cpp -o uploadwpa

or

1. Compile uploadwpa cli for pineapple nano & MK5 (mips architecture)
  Get MK5 firmware and cross compilation toolkit here: http://wiki.wifipineapple.com/#!build_guide.md
  Once set up and installed, from base directory execute 'make menuconfig', select "Base system", scroll down to libstdcpp and hit 'Y' exit and save.
  Copy all files to and create new directory package/uploadwpa 
  Execute 'make menuconfig' once again, scroll down to utilities, find newly listed 'uploadwpa' package and hit 'M', save and exit
  Finally execute 'make package/uploadwpa/compile' and find newly created uploadwpa_1_ar71xx.ipk in bin/ar71xx/packages/
  Copy that to your MK5 or nano and do opkg install uploadwpa_1_ar71xx.ipk to install it! :)
  (Tetra instructions coming soon)

2.
	Copy the compiled binary 'uploadwpa' to /usr/bin or /usr/local/bin on your linux system. If pineapple it should be installed already to the proper directory.

3.
	Then copy UploadWPA folder containing the GUI part of the module to /pineapple/modules/ if you're installing on a pineapple!
