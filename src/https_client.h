// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "const.h"
#include "proxy.h"
#include "types.h"
#include "util/logger.h"

#include <Poco/Activity.h>
#include <Poco/Timestamp.h>

namespace Poco {
namespace Net {
class HTMLForm;
} // namespace Poco::Net
} // namespace Poco

namespace toggl {

class TOGGL_INTERNAL_EXPORT ServerStatus {
 public:
    ServerStatus()
        : gone_(false)
    , checker_(this, &ServerStatus::runActivity)
    , fast_retry_(true) {}

    virtual ~ServerStatus() {
        stopStatusCheck("destructor");
    }

    error Status();
    void UpdateStatus(const Poco::Int64 status_code);
    void DisableStatusCheck() {
        stopStatusCheck("DisableStatusCheck");
    }

 protected:
    void runActivity();

 private:
    bool gone_;
    Poco::Activity<ServerStatus> checker_;
    bool fast_retry_;

    void setGone(const bool value);
    bool gone();

    void startStatusCheck();
    void stopStatusCheck(const std::string &reason);
    bool checkingStatus();

    Logger logger() const;
};

class TOGGL_INTERNAL_EXPORT HTTPClientConfig {
 public:
    HTTPClientConfig()
        : AppName("")
    , AppVersion("")
    , UseProxy(false)
    , ProxySettings(Proxy())
    , IgnoreCert(false)
    , CACertPath("")
    , AutodetectProxy(true) {}
    ~HTTPClientConfig() {}

    std::string AppName;
    std::string AppVersion;
    bool UseProxy;
    toggl::Proxy ProxySettings;
    bool IgnoreCert;
    std::string CACertPath;
    bool AutodetectProxy;

    std::vector<Poco::UInt64> OBMExperimentNrs;

    std::string UserAgent() const {
        std::stringstream ss;
        ss << AppName + "/" + AppVersion;
        for (auto it = OBMExperimentNrs.begin();
                it != OBMExperimentNrs.end();
                ++it) {
            ss << "-obm-" << *it;
        }
        return ss.str();
    }
};

class TOGGL_INTERNAL_EXPORT HTTPRequest {
 public:
    HTTPRequest()
        : method("")
    , host("")
    , relative_url("")
    , payload("")
    , basic_auth_username("")
    , basic_auth_password("")
    , form(nullptr)
    , timeout_seconds(kHTTPClientTimeoutSeconds) {}
    virtual ~HTTPRequest() {}

    std::string method;
    std::string host;
    std::string relative_url;
    std::string payload;
    std::string basic_auth_username;
    std::string basic_auth_password;
    Poco::Net::HTMLForm *form;
    Poco::Int64 timeout_seconds;
};

class TOGGL_INTERNAL_EXPORT HTTPResponse {
 public:
    HTTPResponse()
        : body("")
    , err(noError)
    , status_code(0) {}
    virtual ~HTTPResponse() {}

    std::string body;
    error err;
    Poco::Int64 status_code;
};

class TOGGL_INTERNAL_EXPORT HTTPClient {
 public:
    HTTPClient() {}
    virtual ~HTTPClient() {}

    HTTPResponse Post(
        HTTPRequest req) const;

    HTTPResponse Get(
        HTTPRequest req) const;

    HTTPResponse GetFile(
        HTTPRequest req) const;

    HTTPResponse Delete(
        HTTPRequest req) const;

    HTTPResponse Put(
        HTTPRequest req) const;

    static HTTPClientConfig Config;

 protected:
    virtual HTTPResponse request(
        HTTPRequest req) const;

    virtual Logger logger() const;

 private:
    // We only make requests if this timestamp lies in the past.
    static std::map<std::string, Poco::Timestamp> banned_until_;

    error statusCodeToError(const Poco::Int64 status_code) const;

    bool isRedirect(const Poco::Int64 status_code) const;

    virtual HTTPResponse makeHttpRequest(
        HTTPRequest req) const;

    std::string clientIDForRefererHeader() const;
};

class TOGGL_INTERNAL_EXPORT SyncStateMonitor {
 public:
    virtual ~SyncStateMonitor() {}

    virtual void DisplaySyncState(const Poco::Int64 state) = 0;
};

class TOGGL_INTERNAL_EXPORT TogglClient : public HTTPClient {
 public:
    explicit TogglClient(SyncStateMonitor *monitor = nullptr)
        : monitor_(monitor) {}

    static ServerStatus TogglStatus;

 protected:
    virtual HTTPResponse request(
        HTTPRequest req) const override;

    virtual Logger logger() const override;

 private:
    SyncStateMonitor *monitor_;
};

}  // namespace toggl

#endif  // SRC_HTTPS_CLIENT_H_
