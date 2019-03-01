
const char *SGraphQL::QUERY = "query";
const char *SGraphQL::MUTATION = "mutaion";
const char *SGraphQL::ACTION = "action";
const char *SGraphQL::UNDEFINED = "undefined";
const char *SGraphQL::ALL = "all";

const char *SGraphQL::QUERY_SECTION = "data";

SGraphQL::SGraphQL() : handlers{} {}

SGraphQL::~SGraphQL() { handlers.clear(); }

bool SGraphQL::parse(const JsonObject &json, JsonArray & outJson) {
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
	// JsonObject& data = json.get<JsonObject&>("Data");
	emit(type, component, json, outJson);

	logger.debug("Json API request parsed success\n");
	return true;
}

const ApiHandler &SGraphQL::addHandler(const std::shared_ptr<const ApiHandler> handler) {
	handlers.push_back(handler);
	return *handler;
}

bool SGraphQL::removeHandler(const std::shared_ptr<const ApiHandler> handler) {
	const size_t size = handlers.size();
	handlers.remove(handler);
	return size > handlers.size();
}

const ApiHandler &SGraphQL::on(const char *component, const char *type, ApiHandlerFunction cb) {
	std::shared_ptr<ApiHandler> handler(new ApiHandler(component, type, cb));
	addHandler(handler);
	return *handler;
}

void SGraphQL::emit(const char *event, const char *component, const JsonObject &json, JsonArray & outJson) {
	logger.debug("Start emmit for event: %s, component: %s, \n", event, component);
	for (const auto &h : handlers) {
		logger.debug("Component: %s, handler component: %s\n", component, h->component);
		if (utils::streq(h->component, component)) {
			logger.debug("Event: %s, handler type: %s\n", event, h->type);
			if (utils::streq(h->type, SGraphQL::ALL) || utils::streq(h->type, event)) {
				h->callback_fn(event, json, outJson);
			}
		}
	}
}

ApiHandler::ApiHandler(const char *component, const char *event, ApiHandlerFunction callback) : callback_fn{callback} {

	this->type = new char[sizeof(event)];
	strcpy(this->type, event);

	this->component = new char[sizeof(component)];
	strcpy(this->component, component);
}

ApiHandler::~ApiHandler() {
	logger.debug("Destroy ApiHandler\n");
	if (type) {
		logger.debug("Delete [] event: %s\n", type);
		delete[] type;
	}
	if (component) {
		logger.debug("Delete [] component: %s\n", component);
		delete[] component;
	}
}