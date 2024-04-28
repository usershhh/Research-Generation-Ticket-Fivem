#include <iostream>
#include <string>
#include <curl/curl.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <nlohmann/json.hpp>
#include <thread>


using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

int main() {
    std::string token = boost::uuids::to_string(boost::uuids::random_generator()());

    std::cout << "cfxToken Client : " << token << std::endl;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Erreur lors de l'initialisation de cURL." << std::endl;
        return 1;
    }

    std::string url = "https://cfx.re/api/register/?v=2";

    std::string jsonContent = R"({
        "token": "anonymous",
        "tokenEx": ")" + token + R"(",
        "port": "443"
    })";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonContent.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonContent.size());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: CitizenFX/1");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        std::cout << "\nToken envoyé avec succès à l'API." << std::endl;
        json jsonResponse = json::parse(responseData);
        std::string rpToken = jsonResponse["rpToken"];
        std::string host = jsonResponse["host"];
        std::cout << "rpToken : " << rpToken << std::endl;
        std::cout << "host : " << host << std::endl;

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        while (true) {
            std::string url2 = "https://cnl-hb-live.fivem.net/api/validate/entitlement";
            CURL* curl = curl_easy_init();

            if (!curl) {
                std::cerr << "Erreur lors de l'initialisation de cURL." << std::endl;
                return 1;
            }
            
            std::string entitlementId = "0123456789"; // id de droit
            std::string gameName = "gta5";
            std::string h2 = "0123456789"; // hwid
            std::string rosId = "0123456789"; // id random
            std::string i = "0123456789"; // inconnu
            std::string machineHash = "123456789"; // machine hash
            std::string machineHashIndex = "123456789"; // machine hash index

            std::string postFields = "entitlementId=" + entitlementId + "&gameName=" + gameName + "&h2=" + h2 + "&rosId=" + rosId + "&i=" + i + "&machineHash=" + machineHash + "&machineHashIndex=" + machineHashIndex;

            curl_easy_setopt(curl, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.size());

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "User-Agent: CitizenFX/1");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string responseData;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

            CURLcode res = curl_easy_perform(curl);

            if (res == CURLE_OK) {
                std::cout << "Réponse de l'API" << std::endl;
            } else {
                std::cerr << "Échec de la requête." << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }

    } else {
        std::cerr << "\nÉchec de l'envoi du token à l'API. Erreur : " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
