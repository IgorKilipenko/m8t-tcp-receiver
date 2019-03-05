#ifndef sgraphql_h
#define sgraphql_h

#include "ArduinoJson.h"
#include "utils.h"
#include <list>
#include <queue>
#include <memory>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#ifdef ESP32
#include <AsyncWebServer.h>
#elif defined(ESP8266)
#include <ESPAsyncWebServer.h>
#else
#error Platform not supported
#endif

class ApiHandler;
class ApiResult;

typedef std::shared_ptr<const ApiHandler> ApiHandlerPtr;
typedef std::shared_ptr<ApiResult> ApiResultPtr;

typedef std::function<uint8_t(AsyncWebServerRequest *)> ApiThenAction;
typedef std::function<ApiResultPtr(const char *event, const JsonObject &json, JsonObject &outJson)> ApiHandlerFunction;

class SGraphQL {
  public:
	static const char *QUERY;
	static const char *MUTATION;
	static const char *ACTION;
	static const char *UNDEFINED;
	static const char *ALL;

	static const char *WIFI;
	static const char *GPS;

	static const char *RESP_ROOT_NAME;

	SGraphQL();
	~SGraphQL();
	ApiResultPtr parse(const JsonObject &, JsonObject &);
	ApiHandler &on(const char *, const char *, ApiHandlerFunction);
	ApiHandler &addHandler(const std::shared_ptr<ApiHandler>);
	bool removeHandler(const std::shared_ptr<ApiHandler>);

	static JsonObject &fillRootObject(const char *request_id, JsonObject &outJson);
	static bool validRequest(const JsonObject &json);

  private:
	std::list<ApiHandlerPtr> handlers;
	ApiResultPtr emit(const char *event, const char *component, const JsonObject &, JsonObject &);
};

class ApiHandler {
	// friend SGraphQL;

  public:
	ApiHandler(const char *component, const char *event, ApiHandlerFunction callback);
	~ApiHandler();
	char *getEventName() const;
	char *getComponentName() const;
	ApiHandlerFunction getCallback() const;
	bool test(const char *event, const char *component) const;

  private:
	ApiHandlerFunction callback_fn;
	char *_type;
	char *_component;
};

class ApiResult {
  public:
	ApiResult();
	~ApiResult();
	const JsonObject &getJsonResult() const;
	/** Execute after Reques-> send, return err code */
	uint8_t then(AsyncWebServerRequest *);
	void addAction(ApiThenAction action) { _actions.push(action); }
	size_t actionCount() const { return _actions.size(); }

  private:
	size_t _res_count;
	std::queue<ApiThenAction> _actions;
};

#endif