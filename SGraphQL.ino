
const char *SGraphQL::QUERY = "query";
const char *SGraphQL::MUTATION = "mutaion";
const char *SGraphQL::ACTION = "action";
const char *SGraphQL::UNDEFINED = "undefined";
const char *SGraphQL::ALL = "all";

const char *SGraphQL::WIFI = "wifi"; // WiFi Component
const char *SGraphQL::GPS = "gps";   // GPS receiver Conponent

const char *SGraphQL::QUERY_SECTION = "data";

SGraphQL::SGraphQL() : handlers{} {}

SGraphQL::~SGraphQL() { handlers.clear(); }

bool SGraphQL::parse(const JsonObject &json, JsonArray &outJson) {
	if (&json && !json.containsKey("type")) {
		logger.debug("JsonObject not contain key \"type\"\n");
		return false;
	}
	const char *type = json.get<const char *>("type");
	logger.debug("parse -> type = %s\n", type);

	if (!utils::streq(type, QUERY) && !utils::streq(type, MUTATION) && !utils::streq(type, ACTION)) {
		logger.debug("Gql type filed\n");
		return false;
	}

	const char *component = json.get<char *>("component");
	logger.debug("parse -> component = %s\n", component);
	// JsonObject& data = json.get<JsonObject&>("Data");
	uint8_t hcount = emit(type, component, json, outJson);

	logger.debug("Json API request parsed success, hendler count : {%s}\n", hcount);

	return hcount;
}

ApiHandler &SGraphQL::addHandler(const std::shared_ptr<ApiHandler> handler) {
	logger.debug("addHandler -> Add Handler for component: %s, type: %s\n", handler->getComponentName(), handler->getEventName());
	handlers.push_back(handler);
	return *handler;
}

bool SGraphQL::removeHandler(const std::shared_ptr<ApiHandler> handler) {
	const size_t size = handlers.size();
	handlers.remove(handler);
	return size > handlers.size();
}

ApiHandler &SGraphQL::on(const char *component, const char *type, ApiHandlerFunction cb) {
	logger.debug("on -> Start, for component: %s, type: %s\n", component, type);
	std::shared_ptr<ApiHandler> handler(new ApiHandler(component, type, cb));
	addHandler(handler);
	return *handler;
}

uint8_t SGraphQL::emit(const char *event, const char *component, const JsonObject &json, JsonArray &outJson) {
	logger.debug("emit -> Start emit for event: %s, component: %s, \n", event, component);

	uint8_t handlerCount = 0;
	uint8_t i = 0;
	for (const auto &h : handlers) {
		const char * h_component = h->getComponentName();
		const char * h_event = h->getEventName();
		logger.debug("\n emit -> iter: {%i} Component: %s, handler component: %s\n", i++, component, h_component);
		if (utils::streq(h_component, component)) {
			logger.debug("emit -> Event: %s, handler type: %s\n", event, h_event);
			if (utils::streq(h_event, SGraphQL::ALL) || utils::streq(h_event, event)) {
				logger.debug("emit -> Start callback. Component: %s, handler component: %s\n", component, h_component);
				h->getCallback()(event, json, outJson);
				handlerCount++;
			}
		}
	}

	return handlerCount;
}

ApiHandler::ApiHandler(const char *component, const char *event, ApiHandlerFunction callback) : callback_fn{callback} {
	logger.debug("ApiHandle ctr -> component : %s\n", component);
	//this->type = new char[sizeof(event)];
	//strcpy(this->type, event);

	this->type = utils::copynewstr(event);

	//this->component = new char[sizeof(component)];
	//strcpy(this->component, component);
	//logger.debug("ApiHandle ctr -> component : %s\n", this->component);
	
	//this->component = new char[5]{};
	//strcpy(this->component, "wifi");
	this->component = utils::copynewstr(component);
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

char *ApiHandler::getEventName() const{
	return type;
}

char *ApiHandler::getComponentName() const{
	return component;
}

ApiHandlerFunction ApiHandler::getCallback() const{
	return callback_fn;
}