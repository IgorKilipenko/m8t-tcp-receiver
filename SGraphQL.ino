
const char *SGraphQL::QUERY_TYPE = "query";
const char *SGraphQL::MUTATION_TYPE = "mutaion";
const char *SGraphQL::EXEC_TYPE = "exec";
const char *SGraphQL::UNDEFINED_TYPE = "undefined";

SGraphQL::SGraphQL() : handlers(LinkedList<ApiHandler *>([](ApiHandler *h) { delete h; })) {}

SGraphQL::~SGraphQL() { handlers.free(); }

bool SGraphQL::parse(JsonObject &json) {
	if (!json.containsKey("type")) {
		logger.debug("JsonObject not contain key \"type\"\n");
		return false;
	}
	const char *type = json.get<const char *>("type");
    logger.debug("TYPE: %s\n", type);
	if (!strcmp(type,QUERY_TYPE) && !strcmp(type, MUTATION_TYPE) && !strcmp(type, EXEC_TYPE)) {
		logger.debug("Gql type filed\n");
		return false;
	}

	const char *component = json.get<char *>("component");
	emit(type, component, json);

	return true;
}

ApiHandler &SGraphQL::addHandler(ApiHandler *handler) {
	handlers.add(handler);
	return *handler;
}

bool SGraphQL::removeHandler(ApiHandler *handler) { return handlers.remove(handler) ? true : false; }

ApiHandler &SGraphQL::on(const char *component, const char *type, ApiHandlerFunction cb) {
	ApiHandler *handler = new ApiHandler();
	handler->component = component;
	handler->type = type;
	handler->callback_fn = cb;
	addHandler(handler);
	return *handler;
}

void SGraphQL::emit(const char *event, const char *component, JsonObject &res) {
	for (const auto &h : handlers) {
		if (strcmp(h->type, event) && strcmp(h->component, component)) {
			h->callback_fn(res);
		}
	}
}