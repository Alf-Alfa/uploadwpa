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

1. Compile uploadwpa cli:
	g++ -std=c++0x HTTPClient.cpp uploadwpa.cpp -o uploadwpa

You'll need to either cross compile it or have g++ or some kind of recent c++ compiler if you want it to be a pineapple module otherwise it'll be a linux application!

2.
	Copy the compiled binary 'uploadwpa' to /usr/bin or /usr/local/bin on your pineapple, or linux system.

3.
	Then copy UploadWPA folder containing the GUI part of the module to /pineapple/modules/ if you're installing on a pineapple!
