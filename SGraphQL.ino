
const char *SGraphQL::QUERY = "query";
const char *SGraphQL::MUTATION = "mutaion";
const char *SGraphQL::ACTION = "action";
const char *SGraphQL::UNDEFINED = "undefined";
const char *SGraphQL::ALL = "all";

const char *SGraphQL::QUERY_SECTION = "data";

SGraphQL::SGraphQL() : handlers(LinkedList<ApiHandler *>([](ApiHandler *h) { delete h; })) {}

SGraphQL::~SGraphQL() { handlers.free(); }

bool SGraphQL::parse(const JsonObject &json) {
	if (&json && !json.containsKey("type")) {
		logger.debug("JsonObject not contain key \"type\"\n");
		return false;
	}
	const char *type = json.get<const char *>("type");
	logger.debug("TYPE: %s\n", type);
	if (!utils::streq(type, QUERY) && !utils::streq(type, MUTATION) && !utils::streq(type, ACTION)) {
		logger.debug("Gql type filed\n");
		return false;
	}

	const char *component = json.get<char *>("component");
	//JsonObject& data = json.get<JsonObject&>("Data");
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

void SGraphQL::emit(const char *event, const char *component, const JsonObject &json) {
	for (const auto &h : handlers) {
		logger.debug("Component: %s, handler component: %s\n", component, h->component);
		if (utils::streq(h->component, component)) {
			logger.debug("Event: %s, handler type: %s\n", event, h->type);
			if (utils::streq(h->type, SGraphQL::ALL) || utils::streq(h->type, event)) {
				h->callback_fn(event, json);
			}
		}
	}
}