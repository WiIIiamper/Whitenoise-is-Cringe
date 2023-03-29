#define NOMINMAX
#include "Application.h"
#include "WorkingDirectory.h"
#define CURL_STATICLIB
#include <curl/curl.h>

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    Application app;
    app.Run();

    curl_global_cleanup();
    return 0;
}