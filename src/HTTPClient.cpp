#include "HTTPClient.hpp"

bool HTTPClient::Connect(const char *host, int port)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		Log("ERROR opening socket");
		return false;
	}

	server = gethostbyname(host);
	if (server == 0)
	{
		Log("ERROR no such host");
		return false;
	}

	bzero((char*)&serverAddress, sizeof(serverAddress));
	bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);

	if(connect(sock,(struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		Log("ERROR connecting");
		return false;
	}

	this->host = host;
	this->port = port;
	setReferer("http://" + this->host + "/"); //default referer
	return true;
}

bool HTTPClient::Get(std::string page)
{
    if(page.empty()) return false;

	request = "GET " + page + " HTTP/1.1\r\n";
	if(!host.empty())
		request += "Host: " + host + "\r\n";
	request += "User-Agent: " + userAgent + "\r\n";
	request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	request += "Accept-Language: en-US,en;q=0.5\r\n";
	request += "Accept-Encoding: " + acceptEncoding + "\r\n";
	if(!referer.empty())
		request += "Referer: " + referer + "\r\n";
	request += "Connection: keep-alive\r\n\r\n";
	requestHeaders = request;
	Log("\n[Request: ]\n" + request);

	return WriteRequestReadResponse();
}

bool HTTPClient::Post(std::string page, std::string &data)
{
    if(page.empty() || data.empty()) return false;

    request = "POST " + page + " HTTP/1.1\r\n";
    if(!host.empty())
		request += "Host: " + host + "\r\n";
	request += "User-Agent: " + userAgent + "\r\n";
	request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	request += "Accept-Language: en-US,en;q=0.5\r\n";
	request += "Accept-Encoding: " + acceptEncoding + "\r\n";
	if(!referer.empty())
		request += "Referer: " + referer + "\r\n";
	request += "Connection: keep-alive\r\n";
	request += "Content-Type: application/x-www-form-urlencoded\r\n";
	request += "Content-Length: " + std::to_string(data.size()) + "\r\n\r\n";
	requestHeaders = request;
	request += data;
	request += "\r\n\r\n";
	Log("\n[Request: ]\n" + request);

	return WriteRequestReadResponse();
}

bool HTTPClient::PostMultiPart(std::string page, std::string &data, std::string boundary)
{
    if(page.empty() || data.empty()) return false;

    request = "POST " + page + " HTTP/1.1\r\n";
    if(!host.empty())
		request += "Host: " + host + "\r\n";
	request += "User-Agent: " + userAgent + "\r\n";
	request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	request += "Accept-Language: en-US,en;q=0.5\r\n";
	request += "Accept-Encoding: " + acceptEncoding + "\r\n";
	if(!referer.empty())
		request += "Referer: " + referer + "\r\n";
	request += "Connection: keep-alive\r\n";
	request += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
	request += "Content-Length: " + std::to_string(data.size()) + "\r\n\r\n";

	requestHeaders = request;
    Log("\n[Request: ]\n" + request);

	request += data;
	request += "\r\n\r\n";

	return WriteRequestReadResponse();
}

bool HTTPClient::WriteRequestReadResponse()
{
    int wroteNum = Write((char*)request.c_str(), request.size());
	if(wroteNum > 0)
	{
		int readNum = Read(4096);
		if(readNum > 0)
		{
            interpretResponse();
            return true;
        }
	}
	return false;
}

size_t HTTPClient::interpretResponse()
{
    char *startOfHeader = response.get();
    char *endOfHeader = strstr(startOfHeader, "\r\n\r\n");
    if(endOfHeader == 0) return 0;

    size_t headerSize = (endOfHeader - startOfHeader) + 4;
    responseHeaders.resize(headerSize);
    bcopy(response.get(), (void*)responseHeaders.data(), headerSize);
    dataStartOffset = offset = headerSize;

    Log("\n[Response Headers: ]\n" + responseHeaders);
    //more work needs to be done here, implement chunked transfer encoding...
    if(responseHeaders.find("Transfer-Encoding: chunked") != std::string::npos)
    {
        Log("Chunked Transfer Encoding... {Not yet implemented}");
    }
    else
    {
        size_t pos = responseHeaders.find("Content-Length:");
        if(pos != std::string::npos)
        {
            pos += 15;
            while(responseHeaders[pos] == ' ') { pos++; }
            size_t posEnd = responseHeaders.find("\r\n", pos);
            std::string contentLength = responseHeaders.substr(pos, (posEnd - pos));
            size_t contentLen = strtoull(contentLength.c_str(), 0, 10);
            size_t bytesReceived = responseSize - offset;
            size_t bytesRemaining = contentLen - bytesReceived;
            Log("bytesReceived: " + std::to_string(bytesReceived) + ", bytesRemaining: " + std::to_string(bytesRemaining));

            while(bytesRemaining > 0)
            {
                int readNum = Read(responseSize + bytesRemaining);
                Log("Read # bytes: " + std::to_string(readNum));
                if(readNum > 0)
                {
                    bytesRemaining -= readNum;
                }
                else
                    break;
            }
        }

        Log("\n[Data: ]");
        Log(&response.get()[dataStartOffset]);
    }

    return responseSize;
}

void HTTPClient::clearResponse()
{
    offset = responseSize = 0;
    bzero(response.get(), bufferSize);
}

int HTTPClient::Read(int maxBytes)
{
	if(!response.get() || (bufferSize < maxBytes))
	{
        std::unique_ptr<char[]> oldResponse;
        if(response.get())
        {
            oldResponse = std::move(response);
            maxBytes += responseSize;
        }
		response = std::unique_ptr<char[]>(new char[maxBytes]);
		bufferSize = maxBytes;
		offset = responseSize;
		bcopy(oldResponse.get(), response.get(), responseSize);
	}
	if(!response.get()) return 0;
	clearResponse();

	Log("receiving...");
	int num = recv(sock, &response.get()[offset], (maxBytes - offset), 0);
	if(num <= 0)
	{
		Log("ERROR reading from socket");
		return 0;
	}

    responseSize += num;
	return num;
}

int HTTPClient::Write(char *writeBuffer, int writeSize)
{
    int num, total = 0, bytesLeft = writeSize;

    while(total < writeSize)
    {
        num = send(sock, writeBuffer+total, bytesLeft, 0);
        if(num <= 0) break;
        total += num;
        bytesLeft -= num;
    }

	if(total < writeSize)
	{
		Log("ERROR writing to socket");
		return 0;
	}

	return total;
}

std::string HTTPClient::getRandomBoundary()
{
    std::string randomBoundary = "---------------------------";
    {
        uint64_t random128bits[2];
        FILE *randomness = fopen("/dev/urandom", "rb");
        if(randomness)
        {
            fread(&random128bits[0], 16, 1, randomness);
            fclose(randomness);
            for(int i = 0; i < 2; i++)
            {
                char randomNum[30];
                sprintf(randomNum, "%" PRIu64 "",random128bits[i]);
                randomBoundary += randomNum;
            }


            int losehowmanydigits = rand() % 3 + 11;
            randomBoundary.resize(randomBoundary.size()-losehowmanydigits);
            //resulting in a 23 - 29 digit number appended to 27 dashes (like how iceweasal does it)
            return randomBoundary;
        }
    }
    return "";
}

std::string HTTPClient::urlEncode(std::string Str)
{
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = Str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++)
    {
        c = chars[i];
        ic = c;
        // uncomment this if you want to encode spaces with +
        /*if (c==' ') new_str += '+';
        else */
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
        else
        {
            sprintf(bufHex,"%X",c);
            if(ic < 16)
                new_str += "%0";
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
 }

std::string HTTPClient::urlDecode(std::string Str)
{
    std::string ret;
    char ch;
    int i, ii, len = Str.length();

    for (i=0; i < len; i++)
    {
        if(Str[i] != '%')
        {
            if(Str[i] == '+')
                ret += ' ';
            else
                ret += Str[i];
        }
        else
        {
            sscanf(Str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}

template<class T> void HTTPClient::Log(T Str, bool noNewline)
{
    if(verbosity > 0)
    {
        std::cout << Str;
        if(!noNewline) std::cout << "\n";
    }
}

template<class T> void HTTPClient::Log(T Str)
{
    Log(Str, false);
}
