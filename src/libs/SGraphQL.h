#ifndef sgraphql_h
#define sgraphql_h

#include "Arduino.h"
#include <ESPAsyncWebServer.h>
#include "ArduinoJson.h"
#include <list>
#include <queue>
#include <memory>
#include "AsyncJson.h"
#include "utils.h"

class ApiHandler;
class ApiResult;
class ApiEvent;

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
	static const char *CMD;
	static const char *GPS;
	static const char *NTRIP;
	static const char *SERVER;

	static const char *RESP_ERR;
	static const char *RESP_ID;
	static const char *RESP_REQ_ID;
	static const char *RESP_VALUE;
	static const char *RESP_MSG;
	static const char *CMD_START;
	static const char *CMD_STOP;

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

class ApiEvent {
  public:
	ApiEvent(const JsonObject *requestJson, JsonObject *responseJson) : _requestJson{requestJson}, _responseJson{responseJson} {}
	~ApiEvent() {}
	const JsonObject &getRequestJson() const { return *_requestJson; }
	JsonObject &getResponseJson() const { return *_responseJson; }

  private:
	const JsonObject *_requestJson;
	JsonObject *_responseJson;
};

#endif