/* uploadwpa 2.0 ~ AlfAlfa */
#include "HTTPSClient.hpp"
#include "DefaultConfig.hpp"
#include <sys/stat.h>
#include <ftw.h>
#include <fnmatch.h>
#include <fstream>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;
using str = std::string;

class FormUrl
{
public:
    str url, host, path;
    int port;
    bool isSSL;

    FormUrl() { isSSL = false; }
    FormUrl(str formUrl) { url = formUrl; interpret(); }

    void getHost()
    {
        size_t e = path.find('/');
        if(e != str::npos)
        {
            host = path.substr(0, e);
            path = path.substr(e);
        }
    }
    void interpret(str thisUrl = "")
    {
        if(!thisUrl.empty()) url = thisUrl;

        size_t i = url.find("https://");
        if(i != str::npos)
        {
            path = url.substr(i+8);
            getHost();
            port = 443;
            isSSL = true;
        }
        else
        {
            i = url.find("http://");
            if(i != str::npos)
            {
                path = url.substr(i+7);
                getHost();
                port = 80;
                isSSL = false;
            }
        }
    }
};

class ProcessExecutor
{
private:
	static std::unique_ptr<ProcessExecutor> mainInstance;
public:
	FILE *file;
	std::string output;
	char buffer[4096];

	ProcessExecutor() { memset(buffer, 0, sizeof(buffer)); }
	static std::unique_ptr<ProcessExecutor> make()
	{
		return std::unique_ptr<ProcessExecutor>(new ProcessExecutor());
	}
	static ProcessExecutor *get()
	{
		if(!mainInstance.get()) mainInstance = make();
		return mainInstance.get();
	}
	int run(str cmd, bool printout = false)
	{
		file = popen(cmd.c_str(),"r");
		if(!file) return 1;

		output.clear();
		while(fgets(buffer, sizeof(buffer), file) != 0)
		{
			output += buffer;
			if(printout) std::cout << buffer;
		}
		if(printout) std::cout << "\n";

		pclose(file);
		return 0;
	}
};
std::unique_ptr<ProcessExecutor> ProcessExecutor::mainInstance;

class WebApp
{
private:
    static std::unique_ptr<WebApp> mainInstance;
    FormUrl url;
    json j;
    bool toAllSites = false, skipHandshakeCheck = false, skipSSLValidation = false, usingUploadDir = false;

public:
    std::vector<str> hashes, selectedSites, selectedExts;
    std::string uploadDir;
    str email, essid, bssid, nextHashes, cap_file, cfg_file, file_name, boundary, userAgent, referer, acceptEncoding, hashesSeparator, postData;
    size_t hashesLimit = 0;
    int successCount = 0, attempts = 0, verbosity = 0;

    WebApp() { cfg_file = "~/.uploadwpa2/sites.cfg"; hashesSeparator = "\r\n"; successCount = 0; }
    static std::unique_ptr<WebApp> make() { return std::unique_ptr<WebApp>(new WebApp()); }
    static WebApp *get()
	{
		if(!mainInstance.get()) mainInstance = make();
		return mainInstance.get();
	}
    bool isUsingUploadDir() { return usingUploadDir; }
    void setUsingUploadDir(bool yesorno) { usingUploadDir = yesorno; }
    void setCapFile(str capPath) { cap_file = capPath; file_name = fileNameFromPath(capPath); }
    int doUploads()
    {
        for(auto site : j["sites"])
        {
            if(isSelectedSite(site["name"]))
            {
                std::cout << "Currently selected site:" << site["name"] << "\n";
                if(site["disabled"] != nullptr && site["disabled"] == true)
                    continue;
                if(!isUsingUploadDir() && hashes.size() > 0 && site["postHashesData"] != nullptr)
                    postHashes(site["postHashesData"]);
                if(!cap_file.empty() && site["postWPAHandshakeData"] != nullptr)
                    postHandshakes(site["postWPAHandshakeData"]);
            }
        }
        return successCount;
    }

    void getSpecificData(json &jsonArray)
    {
        if(jsonArray["form"] != nullptr) url.interpret(jsonArray["form"]);
        if(jsonArray["port"] != nullptr) url.port = jsonArray["port"];
        if(jsonArray["referer"] != nullptr) referer = jsonArray["referer"];
        if(jsonArray["userAgent"] != nullptr) userAgent = jsonArray["userAgent"];
        if(jsonArray["acceptEncoding"] != nullptr) acceptEncoding = jsonArray["acceptEncoding"];
        if(jsonArray["hashesSeparator"] != nullptr) hashesSeparator = jsonArray["hashesSeparator"];
    }

    void postHashes(json &hashesData)
    {
        getSpecificData(hashesData);
        if(hashesData["hashesPerPost"] != nullptr) hashesLimit = hashesData["hashesPerPost"];

        auto multipartData = hashesData["multipartData"];
        if(multipartData != nullptr)
        {
            submitMultipartForm(multipartData);
            return;
        }

        auto vars = hashesData["vars"];
        if(vars != nullptr)
        {
            size_t numPosts = (hashes.size() / hashesLimit);
            size_t numHashesLeft = hashes.size();
            if((hashesLimit * numPosts) != numHashesLeft) numPosts++;

            if(verbosity > 0) std::cout << "# of posts to make: " << numPosts << ", total hashes to post: " << numHashesLeft << "\n";

            for(size_t z = 0; z < numPosts; z++)
            {
                if(numHashesLeft <= hashesLimit)
                    nextHashes = makeHashesStringFromOffset(z, numHashesLeft);
                else
                    nextHashes = makeHashesStringFromOffset(z, hashesLimit);

                numHashesLeft -= hashesLimit;
                attempts++;
                successCount += submitForm(vars);
            }
        }
    }

    void postHandshakes(json &handshakeData)
    {
        getSpecificData(handshakeData);

        auto multipartData = handshakeData["multipartData"];
        if(multipartData != nullptr)
            submitMultipartForm(multipartData);
    }

    bool submitForm(json &vars)
    {
        postData.clear();
        for(size_t i = 0; i < vars.size(); i++)
        {
            if(i > 0) postData += "&";
            auto keyValue = vars[i].get<std::unordered_map<str, json>>();

            for(auto x : keyValue)
                postData += x.first + "=" + HTTPClient::urlEncode(returnRealFromPlaceholder(x.second));
        }
        return submitPost();
    }

    bool submitMultipartForm(json &multipart)
    {
        boundary = HTTPClient::getRandomBoundary();
        if(hashes.size() > 0) nextHashes = makeHashesStringFromOffset(0, hashes.size());
        postData.clear();
        auto parts = multipart[0];
        for(size_t i = 0; i < parts.size(); i++)
        {
            auto part = parts[std::to_string(i)];
            if(part != nullptr)
            {
                postData += "--" + boundary + "\r\nContent-Disposition: form-data; ";
                for(size_t x = 0; x < part.size(); x++)
                {
                    auto keyValue = part[x].get<std::unordered_map<str, json>>();
                    for(auto x : keyValue)
                    {
                        if(x.first == "name")
                            postData += "name=\"" + returnRealFromPlaceholder(x.second) + "\"";
                        else if(x.first == "contentType")
                        {
                            postData += "\r\nContent-Type: ";
                            postData += x.second;
                        }
                        else if(x.first == "data")
                            if(x.second == "$filedata")
                                loadCaptureIntoPostData();
                            else
                                postData += "\r\n\r\n" + returnRealFromPlaceholder(x.second) + "\r\n";
                        else
                            postData += "; " + x.first + "=\"" + returnRealFromPlaceholder(x.second) + "\"";
                    }
                }
            }
        }
        postData += boundary + "--\r\n\r\n";
        attempts++;
        successCount += submitPost(true);
        return true;
    }


    bool submitPost(bool multipart = false)
    {
        if(url.isSSL)
        {
            std::unique_ptr<HTTPSClient> https(new HTTPSClient());
            https->setVerbosity(verbosity);
            https->setAcceptEncoding(acceptEncoding);
            https->setUserAgent(userAgent);
            https->requireVerification = !skipSSLValidation;
            if(!https->Connect(url.host.c_str(), url.port)) return false;
            https->setReferer(referer);

            if(multipart)
                return https->PostMultiPart(url.path, postData, boundary);
            else
                return https->Post(url.path, postData);
        }
        else
        {
            std::unique_ptr<HTTPClient> http(new HTTPClient());
            http->setVerbosity(verbosity);
            http->setAcceptEncoding(acceptEncoding);
            http->setUserAgent(userAgent);
            if(!http->Connect(url.host.c_str(), url.port)) return false;
            http->setReferer(referer);

            if(multipart)
                return http->PostMultiPart(url.path, postData, boundary);
            else
                return http->Post(url.path, postData);
        }
    }

    bool loadCaptureIntoPostData()
    {
        FILE *file = fopen(cap_file.c_str(), "rb");
        if(!file) { std::cout << "ERROR Cannot open file: \"" << cap_file << "\"...\n"; return false; }
        fseek(file, 0, SEEK_END);
        long fileLen = ftell(file);
        rewind(file);

        postData += "\r\n\r\n";
        size_t previousLen = postData.size();
        postData.resize(previousLen + fileLen);
        fread((void*)&postData.data()[previousLen], fileLen, 1, file);
        fclose(file);
        postData += "\r\n";
        return true;
    }

    bool isSelectedSite(str currentSite)
    {
        if(toAllSites) return true;

        for(size_t i = 0; i < selectedSites.size(); i++)
        {
            if(currentSite == selectedSites[i])
                return true;
        }
        return false;
    }

    str makeHashesStringFromOffset(size_t offset, size_t len)
    {
        str hashesStr;
        for(size_t x = 0, i = offset * hashesLimit; i < ((offset * hashesLimit) + len); i++, ++x)
        {
            hashesStr += hashes[i];
            if(x != (len - 1))
                hashesStr += hashesSeparator;
        }
        return hashesStr;
    }

    bool replaceVars(str &input)
    {
        size_t numReplaced = 0;
        numReplaced += replace(input, "$email", email);
        numReplaced += replace(input, "$filename", file_name);
        numReplaced += replace(input, "$hashes", nextHashes);
        numReplaced += replace(input, "$essid", essid);
        numReplaced += replace(input, "$bssid", bssid);
        if(numReplaced > 0) return true;
        else return false;
    }

    str returnRealFromPlaceholder(str input)
    {
        while(replaceVars(input))
        {
        }
        return input;
    }

    bool replace(str &inThis, const str &usingThis, const str &withThis)
    {
        size_t atOffset = inThis.find(usingThis);
        if(atOffset == str::npos)
            return false;
        inThis.replace(atOffset, usingThis.size(), withThis);
        return true;
    }

    str fileNameFromPath(str filePath)
    {
        size_t lastSlash = filePath.rfind('/');
        if(lastSlash != str::npos)
            return filePath.substr(lastSlash+1);
        return filePath;
    }

    bool handshakeVerify(const str handshakePath)
    {
        if(!skipHandshakeCheck && j["handshakeCheckEnabled"] != nullptr && j["handshakeCheckEnabled"] == true)
        {
            std::cout << "Checking \"" << handshakePath << "\" for valid handshake\n";
            if(j["handshakeCheckCommand"] != nullptr && j["handshakeCheckValid"] != nullptr)
            {
                str checkCommand = j["handshakeCheckCommand"];
                str checkValid = j["handshakeCheckValid"];
                replace(checkCommand, "$filepath", handshakePath);

                auto exe = ProcessExecutor::get();
                if(exe->run(checkCommand.c_str()) == 0)
                {
                    if(exe->output.find(checkValid) != str::npos)
                    {
                        std::cout << "WPA Handshake is good!\n";
                        return true;
                    }
                    std::cout << "Capture file does not contain a WPA Handshake\n";
                }
            }
            return false;
        }
        return true;
    }

    str returnHomePath()
    {
        str usr = getlogin();
        if(usr == "root") //shouldn't really be using this as root, but lets not have it be broken if you are...
            return "/root";
        else
            return "/home/" + usr;
    }

    bool loadCfg(str cfgPath)
    {
        std::ifstream cfg;
        if(cfgPath[0] == '~')
        {
            str homePath = returnHomePath();
            cfgPath.replace(0, 1, homePath);
            homePath += "/.uploadwpa2";
            str defaultPath = homePath + "/sites.cfg";

            if(cfgPath == defaultPath)
            {
                cfg_file = cfgPath;
                cfg.open(cfg_file);
                if(!cfg.is_open())
                {
                    std::cout << "Saving default config... -> " << cfg_file << "\n";
                    mkdir(homePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    saveDefaultConfig(cfg_file);
                    cfg.open(cfg_file);
                }
            }
            else
                cfg.open(cfgPath);
        }
        else
            cfg.open(cfgPath);
        if(cfg.is_open())
        {
            cfg >> j;
            cfg.close();

            str defaultAcceptEncoding = j["acceptEncoding"];
            str defaultUserAgent = j["defaultUserAgent"];
            str defaultSite = j["defaultSite"];

            if(selectedSites.empty() && !defaultSite.empty()) selectedSites.push_back(defaultSite);
            if(!defaultUserAgent.empty()) userAgent = defaultUserAgent;
            if(!defaultAcceptEncoding.empty()) acceptEncoding = defaultAcceptEncoding;
            if(verbosity > 0) std::cout << "Config loaded: " << cfgPath << "\n";
            return true;
        }
        return false;
    }

    bool saveCfg(str cfgPath)
    {
        std::ofstream cfg;
        cfg.open(cfgPath, std::ofstream::out);
        if(cfg.is_open())
        {
            cfg << j.dump(4);
            cfg.close();
            return true;
        }
        return false;
    }

    void saveDefaultConfig(str defaultCfgPath)
    {
        j = DefaultConfig::cfg;
        saveCfg(defaultCfgPath);
    }

    int processArgs(int c, char **args)
    {
        for(int i = 1; i < c; i++)
        {
            if(i < (c - 1))
            {
                if(strcmp(args[i],"-e") == 0 || strcmp(args[i],"--email") == 0)
                {
                    email = args[i+1];
                }
                if(strcmp(args[i],"-es") == 0 || strcmp(args[i],"--essid") == 0)
                {
                    essid = args[i+1];
                }
                if(strcmp(args[i],"-bs") == 0 || strcmp(args[i],"--bssid") == 0)
                {
                    bssid = args[i+1];
                }
                if(strcmp(args[i],"-cf") == 0 || strcmp(args[i],"--config") == 0)
                {
                    cfg_file = args[i+1];
                }
                if(strcmp(args[i],"-d") == 0 || strcmp(args[i],"--dir") == 0)
                {
                    uploadDir = args[i+1];
                    setUsingUploadDir(true);
                }
                if(strcmp(args[i],"-u") == 0 || strcmp(args[i],"--user-agent") == 0)
                {
                    userAgent = args[i+1];
                }
                if(strcmp(args[i],"-c") == 0 || strcmp(args[i],"--cap-file") == 0)
                {
                    setCapFile(args[i+1]);
                }
                if(strcmp(args[i],"-s") == 0 || strcmp(args[i],"--skip") == 0)
                {
                    str skip = args[i+1];
                    if(skip == "1")
                        skipHandshakeCheck = true;
                    else if(skip == "2")
                        skipSSLValidation = true;
                    else if(skip == "3")
                        skipHandshakeCheck = skipSSLValidation = true;
                }
                if(strcmp(args[i],"-a") == 0 || strcmp(args[i],"--hashes") == 0)
                {
                    int z = i;
                    while(*args[++z] != '-')
                    {
                        hashes.push_back(args[z]);
                        if(z == (c - 1) || *args[z+1] == '-') break;
                    }
                }
                if(strcmp(args[i],"-t") == 0 || strcmp(args[i],"--to") == 0)
                {
                    int z = i;
                    while(*args[++z] != '-')
                    {
                        selectedSites.push_back(args[z]);
                        if(z == (c - 1) || *args[z+1] == '-') break;
                    }
                }
                if(strcmp(args[i],"-x") == 0 || strcmp(args[i],"--extensions") == 0)
                {
                    int z = i;
                    while(*args[++z] != '-')
                    {
                        selectedExts.push_back(str("*.") + args[z]);
                        if(z == (c - 1) || *args[z+1] == '-') break;
                    }
                }
            }
            if(strcmp(args[i],"-v") == 0 || strcmp(args[i],"-vv") == 0 || strcmp(args[i],"-vvv") == 0)
            {
                verbosity = 1;
            }
            if(strcmp(args[i],"--all") == 0)
            {
                toAllSites = true;
            }
            if(strcmp(args[i],"-h") == 0 || strcmp(args[i],"--help") == 0)
            {
                return 2;
            }
        }

        if(email.empty() && (hashes.empty() || cap_file.empty())) return 2;
        if(selectedExts.empty()) selectedExts.push_back("*.cap");

        loadCfg(cfg_file);

        if(!isUsingUploadDir() && !cap_file.empty() && !handshakeVerify(cap_file)) return 1;

        return 0;
    }
};
std::unique_ptr<WebApp> WebApp::mainInstance;

void printHelp()
{
	std::cout << "uploadwpa 2.0 ~ AlfAlfa\nBTC: 1GEKnNuVkMeMWiMcTQf8io2GqgCusQcK1E\n";
	std::cout << "This module will upload a single capture file containing wpa handshake to various online hash crackers!\n";
	std::cout << "You can also give it a virtually unlimited number of hashes for sites that support and are configured for this\n";
	std::cout << "-t / --to [sitename1 sitename2] selects which sites from your config file or --all for all sites, defaultSite in config for default\n";
	std::cout << "-c / --cap-file [/path/to/capturefile.cap] specifies single handshake file and -a / --hashes [hash1 hash2] specifies hashes\n";
	std::cout << "-d / --dir [directory] specifies a directory containing entirely handshakes you're sure you want to upload, to all specified and configured sites\n";
    std::cout << "-x / --extensions [extension1 extension2] specify extensions of your handshakes in -d option's directory, default is just: cap\n";
    std::cout << "-cf / --config [/path/to/sites.cfg] uses a json config from a specified path (default cfg at: \"~/.uploadwpa2/sites.cfg\")\n";
    std::cout << "-es / --essid [ESSID] and -bs / --bssid [BSSID] specifies essid/bssid for sites that may require it\n";
    std::cout << "-s / --skip [num] skips checks: 1 == skip wpa handshake validation, 2 == skip SSL/TLS certificate verification, 3 == skip both\n";
    std::cout << "-v / -vv / -vvv sets verbosity to true for verbose output\n";
	std::cout << "Usage:\n";
	std::cout << "{Send WPA Handshake to onlinehashcrack:}\nuploadwpa -t onlinehashcrack -e youremail@yourdomain.com -c path/to/myaccesspoint.cap\n";
	std::cout << "{Send an unlimited number of hashes at 10/somenumber (specified in config) at a time until all are sent of hashes supported by the site:}\n";
	std::cout << "uploadwpa -t onlinehashcrack -e youremail@yourdomain.com -a hash1 hash2 hash3 hash4 etc etc ...\n";
	std::cout << "{Send both sequentially:}\nuploadwpa -t onlinehashcrack -e example@example.com -a hash1 etc etc -c /path/to/capture.cap\n";
	std::cout << "{Send WPA Handshake to cloudcracker:}\nuploadwpa -t cloudcracker -es myAP-essid -e example@example.com -c myaccesspoint.cap\n";
	std::cout << "{Send WPA Handshake to all configured sites:}\nuploadwpa --all -es myAP-essid -bs 11:22:33:44:55:66 -e example@example.com -c myaccesspoint.cap\n";
	std::cout << "{Send all handshakes in dir to all configured sites:}\nuploadwpa -d ~/captures/toupload --all -es myAP -bs 11:22:33:44:55:66 -e example@example.com\n";
}

static int directorySearchCallback(const char *fpath, const struct stat *sb, int typeflag)
{
    auto w = WebApp::get();
    /* if it's a file */
    if(typeflag == FTW_F)
    {
        for(size_t i = 0; i < w->selectedExts.size(); i++)
        {
            /* if the filename matches the filter, */
            if(fnmatch(w->selectedExts[i].c_str(), fpath, FNM_CASEFOLD) == 0)
            {
                if(w->handshakeVerify(fpath))
                {
                    std::cout << "Uploading \"" << fpath << "\"\n";
                    w->setCapFile(fpath);
                    w->doUploads();
                }
                break;
            }
        }
    }
    /* tell ftw to continue */
    return 0;
}

int main(int argcount, char *args[])
{
    auto app = WebApp::get();
    if(app != nullptr)
    {
        int retval = app->processArgs(argcount, args);

        if(retval == 2) { printHelp(); return 2; }
        else if(retval == 0)
        {
            if(app->isUsingUploadDir())
                ftw(app->uploadDir.c_str(), directorySearchCallback, 16);
            else
                app->doUploads();

            if(app->successCount > 0)
            {
                std::cout << "Successful! " << app->successCount << "\n";
                return 0;
            }
            else
                std::cout << "Failed... " << app->attempts << "\n";
        }
    }
    return 1;
}
