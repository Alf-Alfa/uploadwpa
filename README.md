# uploadwpa2

uploadwpa 2.0 ~ AlfAlfa
BTC: 1GEKnNuVkMeMWiMcTQf8io2GqgCusQcK1E
<------------------------------------------------------------------------------------------------->
/* CHANGE LOG */

 -> TLS/SSL Support Added SSLv23 && SSLv3

 -> Now configurable with json config file, which allows easily setting it to work with more sites, no longer a single site supported only and hardcoded...
    *default location: ~/.uploadwpa2/sites.cfg or specified with -cf [config.cfg]
 
 -> Unlimited hashes can now be specified, and it will just send as many post requests as required based on 'hashesPerPost' json configuration for specific site.
    *hashesSeparator also specified in json config, determines what the site expects hashes to be delimited by, default is "\r\n";

 -> Automatically checks to make sure the capture file specified has a valid handshake before sending it out to the site(s) of your choice.
    *customize command that does this with "handshakeCheckCommand" json property. "$filepath" get's replaced  with passed in capture file path,
	and the "handshakeCheckValid" string is what to look for in output when there is a valid handshake.
    *Also "handshakeCheckEnabled" can be set to false to disable this handshake validating, or pass argument -s 1 / --skip 1

 -> More improvements!

This module will upload a single capture file containing wpa handshake to various online hash crackers!
You can also give it a virtually unlimited number of hashes for sites that support and are configured for this
-t / --to [sitename1 sitename2] selects which sites from your config file or --all for all sites, defaultSite in config for default
-c / --cap-file [/path/to/capturefile.cap] specifies single handshake file and -a / --hashes [hash1 hash2] specifies hashes
-d / --dir [directory] specifies a directory containing entirely handshakes you're sure you want to upload, to all specified and configured sites
-x / --extensions [extension1 extension2] specify extensions of your handshakes in -d option's directory, default is just: cap
-cf / --config [/path/to/sites.cfg] uses a json config from a specified path (default cfg at: "~/.uploadwpa2/sites.cfg")
-es / --essid [ESSID] and -bs / --bssid [BSSID] specifies essid/bssid for sites that may require it
-s / --skip [num] skips checks: 1 == skip wpa handshake validation, 2 == skip SSL/TLS certificate verification, 3 == skip both
-v / -vv / -vvv sets verbosity to true for verbose output
Usage:
{Send WPA Handshake to onlinehashcrack:}
uploadwpa -t onlinehashcrack -e youremail@yourdomain.com -c path/to/myaccesspoint.cap
{Send an unlimited number of hashes at 10/somenumber (specified in config) at a time until all are sent of hashes supported by the site:}
uploadwpa -t onlinehashcrack -e youremail@yourdomain.com -a hash1 hash2 hash3 hash4 etc etc ...
{Send both sequentially:}
uploadwpa -t onlinehashcrack -e example@example.com -a hash1 etc etc -c /path/to/capture.cap
{Send WPA Handshake to cloudcracker:}
uploadwpa -t cloudcracker -es myAP-essid -e example@example.com -c myaccesspoint.cap
{Send WPA Handshake to all configured sites:}
uploadwpa --all -es myAP-essid -bs 11:22:33:44:55:66 -e example@example.com -c myaccesspoint.cap
{Send all handshakes in dir to all configured sites:}
uploadwpa -d ~/captures/toupload --all -es myAP -bs 11:22:33:44:55:66 -e example@example.com



1. Compile uploadwpa2 cli for linux:
	g++ -std=c++0x HTTPSClient.cpp HTTPClient.cpp uploadwpa.cpp -o uploadwpa

or

1. Compile uploadwpa cli for pineapple nano && Tetra && MK5 (mips architecture)
  Get OpenWrt SDK here: https://wiki.openwrt.org/doc/howto/obtain.firmware.sdk
  Once set up and installed, from base directory execute 'make menuconfig', select OpenWrt SDK, hit 'Y',
  then select "Base system", scroll down to libstdcpp and hit 'Y' exit and save.
  Copy all files to and create new directory package/utils/uploadwpa2 
  Execute 'make menuconfig' once again, scroll down to utilities, find newly listed 'uploadwpa2' package and hit 'M', save and exit.
  Build the SDK and toolchain with "make" from the openwrt sdk base directory... Wait, then when finished and successful...
  Finally execute 'make package/uploadwpa2/compile' and find newly created uploadwpa2_1_ar71xx.ipk in bin/ar71xx/packages/base
  Copy that to your Tetra, nano, or MK5, and execute: opkg install uploadwpa2_1_ar71xx.ipk to install it! :)

2.
	Copy the compiled binary 'uploadwpa2' to /usr/bin or /usr/local/bin on your linux system, if on a pineapple it should be installed already to the proper directory.

3.
	Then copy UploadWPA folder containing the GUI part of the module to /pineapple/modules/ if you're installing on a pineapple!
