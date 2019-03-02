#ifndef sgraphql_h
#define sgraphql_h

#include "ArduinoJson.h"
#include "utils.h"
#include <list>
#include <memory>

class ApiHandler;

typedef std::function<void(const char* event, const JsonObject &json, JsonArray & outJson)> ApiHandlerFunction;

class SGraphQL {
  public:
	SGraphQL();
	~SGraphQL();
	bool parse(const JsonObject &, JsonArray &);
	ApiHandler &on(const char *, const char *, ApiHandlerFunction);
	ApiHandler &addHandler(const std::shared_ptr<ApiHandler>);
	bool removeHandler(const std::shared_ptr<ApiHandler>);

	static const char *QUERY;
	static const char *MUTATION;
	static const char *ACTION;
	static const char *UNDEFINED;
	static const char *ALL;

	static const char * WIFI;
	static const char * GPS;


	static const char *QUERY_SECTION;

  private:
		std::list<std::shared_ptr<const ApiHandler>> handlers;
    uint8_t emit(const char *event, const char *component, const JsonObject &, JsonArray & );
};

class ApiHandler {
	//friend SGraphQL;

  public:
	ApiHandler(const char* component,const char* event ,ApiHandlerFunction callback);
	~ApiHandler();
	char * getEventName() const;
	char * getComponentName() const;
	ApiHandlerFunction getCallback() const;

  private:
	ApiHandlerFunction callback_fn;
	char *type;
	char *component;

	
};

#endif