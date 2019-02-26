#ifndef sgraphql_h
#define sgraphql_h

#include "ArduinoJson.h"

class ApiHandler;

typedef std::function<void(JsonObject &res)> ApiHandlerFunction;

class SGraphQL {
  public:
	SGraphQL();
	~SGraphQL();
	bool parse(JsonObject &);
	ApiHandler &on(const char *, const char *, ApiHandlerFunction cb);
	ApiHandler &addHandler(ApiHandler *);
	bool removeHandler(ApiHandler *);

	static const char *QUERY_TYPE;
	static const char *MUTATION_TYPE;
	static const char *EXEC_TYPE;
	static const char *UNDEFINED_TYPE;

  private:
	LinkedList<ApiHandler *> handlers;
    void emit(const char *event, const char *component, JsonObject &res);
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