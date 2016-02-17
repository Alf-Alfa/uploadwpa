/* uploadwpa 1.1 ~ AlfAlfa */
#include "HTTPClient.hpp"

std::unique_ptr<HTTPClient> http;
std::string email, hashes, capture_file, file_name, boundary, useThisUserAgent;
int success = 0, verbose = 0;

void printHelp()
{
	std::cout << "uploadwpa 1.0 ~ AlfAlfa\n";
	std::cout << "This module will upload a wpa handshake from a single capture file to www.onlinehashcrack.com\n";
	std::cout << "or you can give it up to 10 hashes seperated by spaces and of any hashes the site supports\nUsage:\n";
	std::cout << "{Send WPA Handshake:}\nuploadwpa -e youremail@yourdomain.com -c myaccesspoint.cap\n";
	std::cout << "uploadwpa -e email@yourdomain.com -c ~/captures/myaccesspoint.hccap -u \"A Custom User Agent\"\n";
	std::cout << "{Send up to 10 hashes at once of hashes supported by the site:}\n";
	std::cout << "uploadwpa -e youremail@yourdomain.com -a hash1 hash2 hash3 hash4 hash5 hash6 hash7 hash8 hash9 hash10\n";
	std::cout << "{Send both sequentially:}\nuploadwpa -e example@example.com -a hash1 etc etc -c /path/to/capture.cap\n";
}

int postHashesTo_onlinehashcrack()
{
    std::string postData = "textareaHashes=";
    postData += HTTPClient::urlEncode(hashes) + "&emailHashes=";
    postData += HTTPClient::urlEncode(email) + "&submit=Submit";

    //if(!http->Connect("127.0.0.1",80)) return 0;
    if(!http->Connect("www.onlinehashcrack.com",80)) return 0;

    bool successful = http->Post("/hash-cracking.php", postData.c_str());
    http->Close();
    return successful;
}

bool postWPAHandshakeTo_onlinehashcrack()
{
    boundary = HTTPClient::getRandomBoundary();

    std::string postData = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"emailWpa\"\r\n\r\n" + email + "\r\n";
    postData += "--" + boundary + "\r\nContent-Disposition: form-data; name=\"wpaFile\"; filename=\"" + file_name + "\"";
    postData += "\r\nContent-Type: application/vnd.tcpdump.pcap\r\n\r\n";

    FILE *file = fopen(capture_file.c_str(), "rb");
    if(!file) { http->Log("ERROR Cannot open file"); return false; }
    fseek(file, 0, SEEK_END);
    long fileLen = ftell(file);
    rewind(file);

    size_t previousLen = postData.size();
    postData.resize(previousLen + fileLen);
    fread((void*)&postData.data()[previousLen], fileLen, 1, file);
    fclose(file);

    postData += "\r\n--" + boundary + "\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nSubmit\r\n";
    postData += boundary + "--\r\n\r\n";

    //if(!http->Connect("127.0.0.1",80)) return false;
    if(!http->Connect("www.onlinehashcrack.com",80)) return false;

    bool successful = http->PostMultiPart("/wifi-wpa-rsna-psk-crack.php", postData, boundary);
    http->Close();
    return successful;
}

int main(int argcount, char *args[])
{
	for(int i = 0; i < argcount; i++)
	{
		if(strcmp(args[i],"-e") == 0 || strcmp(args[i],"--email") == 0)
		{
            if(i < (argcount - 1))
                email = args[i+1];
		}
		if(strcmp(args[i],"-c") == 0 || strcmp(args[i],"--capture-file") == 0)
		{
            if(i < (argcount - 1))
            {
                capture_file = args[i+1];

                size_t lastSlash = capture_file.rfind('/');
                if(lastSlash != std::string::npos)
                    file_name = capture_file.substr(lastSlash+1);
                else
                    file_name = capture_file;
            }
		}
		if(strcmp(args[i],"-a") == 0 || strcmp(args[i],"--hashes") == 0)
		{
            int z = i;
            while(*args[++z] != '-')
            {
                hashes += args[z];
                if(z == (argcount - 1) || *args[z+1] == '-') break;
                hashes += "\r\n";
            }
		}
		if(strcmp(args[i],"-u") == 0 || strcmp(args[i],"--user-agent") == 0)
		{
            if(i < (argcount - 1))
                useThisUserAgent = args[i+1];
		}
		if(strcmp(args[i],"-v") == 0 || strcmp(args[i],"--verbose") == 0)
		{
            verbose = 1;
		}
		if(strcmp(args[i],"-h") == 0 || strcmp(args[i],"--help") == 0)
		{
			printHelp();
			return 2;
		}
	}

	if(!email.empty())
	{
        http = HTTPClient::make();
        if(http.get())
        {
            http->setUserAgent(useThisUserAgent);
            http->setVerbosity(verbose);

            if(!hashes.empty())
                success += postHashesTo_onlinehashcrack();
            if(!capture_file.empty())
                success += postWPAHandshakeTo_onlinehashcrack();

            if(success > 0)
            {
                std::cout << "Successful! " << success << "\n";
                return 0;
            }
        }
        return 1;
	}
    printHelp();
    return 2;
}
