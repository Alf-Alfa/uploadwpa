#include "json.hpp"

class DefaultConfig
{
public:
    static nlohmann::json cfg;
};

nlohmann::json DefaultConfig::cfg = R"(
{
    "acceptEncoding": "gzip, deflate",
    "appVersion": 2.0,
    "defaultSite": "onlinehashcrack",
    "defaultUserAgent": "Mozilla/5.0 (X11; Linux x86_64; rv:38.0) Gecko/20100101 Firefox/38.0 Iceweasel/38.6.0",
    "handshakeCheckCommand": "aircrack-ng \"$filepath\" | grep handshake",
    "handshakeCheckEnabled": true,
    "handshakeCheckValid": "handshake",
    "sites": [
        {
            "name": "onlinehashcrack",
            "postHashesData": {
                "form": "http://www.onlinehashcrack.com/hash-cracking.php",
                "hashesPerPost": 10,
                "hashesSeparator": "\r\n",
                "referer": "http://www.onlinehashcrack.com/",
                "vars": [
                    {
                        "textareaHashes": "$hashes"
                    },
                    {
                        "emailHashes": "$email"
                    },
                    {
                        "submit": "Submit"
                    }
                ]
            },
            "postWPAHandshakeData": {
                "form": "http://www.onlinehashcrack.com/wifi-wpa-rsna-psk-crack.php",
                "multipartData": [
                    {
                        "0": [
                            {
                                "name": "emailWpa"
                            },
                            {
                                "data": "$email"
                            }
                        ],
                        "1": [
                            {
                                "name": "wpaFile"
                            },
                            {
                                "filename": "$filename"
                            },
                            {
                                "contentType": "application/vnd.tcpdump.pcap"
                            },
                            {
                                "data": "$filedata"
                            }
                        ],
                        "2": [
                            {
                                "name": "submit"
                            },
                            {
                                "data": "Submit"
                            }
                        ]
                    }
                ],
                "referer": "http://www.onlinehashcrack.com/"
            }
        },
        {
            "name": "cloudcracker",
            "postWPAHandshakeData": {
                "form": "https://cloudcracker.com/api/wpa/job",
                "multipartData": [
                    {
                        "0": [
                            {
                                "name": "pcap"
                            },
                            {
                                "filename": "$filename"
                            },
                            {
                                "contentType": "application/vnd.tcpdump.pcap"
                            },
                            {
                                "data": "$filedata"
                            }
                        ],
                        "1": [
                            {
                                "name": "essid"
                            },
                            {
                                "data": "$essid"
                            }
                        ],
                        "2": [
                            {
                                "name": "dictionary"
                            },
                            {
                                "data": "english"
                            }
                        ],
                        "3": [
                            {
                                "name": "size"
                            },
                            {
                                "data": "604"
                            }
                        ],
                        "4": [
                            {
                                "name": "email"
                            },
                            {
                                "data": "$email"
                            }
                        ],
                        "5": [
                            {
                                "name": "submit"
                            },
                            {
                                "data": "Submit"
                            }
                        ]
                    }
                ],
                "referer": "https://cloudcracker.com/"
            }
        },
        {
            "name": "test",
			"disabled": false,
            "postHashesData": {
                "acceptEncoding": "identity",
                "form": "http://127.0.0.1/hashes.php",
                "hashesPerPost": 10,
                "hashesSeparator": "\r\n",
                "referer": "http://127.0.0.1/",
                "vars": [
                    {
                        "textareaHashes": "$hashes"
                    },
                    {
                        "emailHashes": "$email"
                    },
                    {
                        "submit": "Submit"
                    }
                ]
            },
            "postWPAHandshakeData": {
                "acceptEncoding": "identity",
                "form": "http://127.0.0.1/handshake.php",
                "multipartData": [
                    {
                        "0": [
                            {
                                "name": "emailWpa"
                            },
                            {
                                "data": "$email"
                            }
                        ],
                        "1": [
                            {
                                "name": "wpaFile"
                            },
                            {
                                "filename": "$filename"
                            },
                            {
                                "contentType": "application/vnd.tcpdump.pcap"
                            },
                            {
                                "data": "$filedata"
                            }
                        ],
                        "2": [
                            {
                                "name": "hashes"
                            },
                            {
                                "data": "$hashes"
                            }
                        ],
                        "3": [
                            {
                                "name": "submit"
                            },
                            {
                                "data": "Submit"
                            }
                        ]
                    }
                ],
                "referer": "http://127.0.0.1/"
            }
        }
    ]
})"_json;
