#ifndef sgraphql_h
#define sgraphql_h

#include "ArduinoJson.h"
#include "utils.h"

class ApiHandler;

typedef std::function<void(const char* event, const JsonObject &json)> ApiHandlerFunction;

class SGraphQL {
  public:
	SGraphQL();
	~SGraphQL();
	bool parse(const JsonObject &);
	ApiHandler &on(const char *, const char *, ApiHandlerFunction cb);
	ApiHandler &addHandler(ApiHandler *);
	bool removeHandler(ApiHandler *);

	static const char *QUERY;
	static const char *MUTATION;
	static const char *ACTION;
	static const char *UNDEFINED;
	static const char *ALL;

	static const char *QUERY_SECTION;

  private:
	LinkedList<ApiHandler *> handlers;
    void emit(const char *event, const char *component, const JsonObject &res);
};

class ApiHandler {
	friend SGraphQL;

  public:
	ApiHandler() {}
	~ApiHandler() {}

  private:
	ApiHandlerFunction callback_fn;
	const char *type;
	const char *component;
};

#endif