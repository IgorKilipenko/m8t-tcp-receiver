#ifndef sgraphql_h
#define sgraphql_h

#include "ArduinoJson.h"
#include "utils.h"
#include <list>
//#include <string>

class ApiHandler;

typedef std::function<void(const char* event, const JsonObject &json)> ApiHandlerFunction;

class SGraphQL {
  public:
	SGraphQL();
	~SGraphQL();
	bool parse(const JsonObject &);
	const ApiHandler &on(const char *, const char *, ApiHandlerFunction);
	const ApiHandler &addHandler(const std::shared_ptr<const ApiHandler>);
	bool removeHandler(const std::shared_ptr<const ApiHandler>);

	static const char *QUERY;
	static const char *MUTATION;
	static const char *ACTION;
	static const char *UNDEFINED;
	static const char *ALL;

	static const char *QUERY_SECTION;

  private:
		std::list<std::shared_ptr<const ApiHandler>> handlers;
    void emit(const char *event, const char *component, const JsonObject &res);
};

class ApiHandler {
	friend SGraphQL;

  public:
	ApiHandler(const char* component,const char* event ,ApiHandlerFunction callback);
	~ApiHandler();

  private:
	ApiHandlerFunction callback_fn;
	char *type;
	char *component;
};

#endif