
const char *SGraphQL::QUERY = "query";
const char *SGraphQL::MUTATION = "mutaion";
const char *SGraphQL::ACTION = "action";
const char *SGraphQL::UNDEFINED = "undefined";
const char *SGraphQL::ALL = "all";
const char *SGraphQL::CMD = "cmd";

const char *SGraphQL::WIFI = "wifi";	// WiFi Component
const char *SGraphQL::GPS = "receiver"; // GPS receiver Conponent
const char *SGraphQL::SERVER = "server";

const char *SGraphQL::RESP_VALUE = "value";
const char *SGraphQL::RESP_MSG = "message";
const char *SGraphQL::RESP_ERR = "error";
const char *SGraphQL::RESP_ID = "resp_id";
const char *SGraphQL::RESP_REQ_ID = "req_id";

const char *SGraphQL::CMD_START = "start";
const char *SGraphQL::CMD_STOP = "stop";

SGraphQL::SGraphQL() : handlers{} {}

SGraphQL::~SGraphQL() { handlers.clear(); }

ApiResultPtr SGraphQL::parse(const JsonObject &json, JsonObject &outJson) {

	if (!validRequest(json)) {
		logger.error("Request Json not vilid\n");
		json.prettyPrintTo(logger);
		logger.print("\n");
		return nullptr;
	}

	const char *type = json.get<const char *>("type");
	logger.debug("parse -> type = %s\n", type);

	const char *component = json.get<char *>("component");
	logger.debug("parse -> component = %s\n", component);

	const char *req_id = json.get<char *>("id");
	logger.debug("parse -> request id = %s\n", req_id);

	JsonObject &responseRoot = fillRootObject(req_id, outJson);

	ApiResultPtr res_ptr = emit(type, component, json, responseRoot);

	if (res_ptr) {
		logger.debug("Json API request parsed success\n");
	}

	return res_ptr;
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

ApiResultPtr SGraphQL::emit(const char *event, const char *component, const JsonObject &json, JsonObject &outJson) {
	logger.debug("emit -> Start emit for event: %s, component: %s, \n", event, component);
	for (const auto &h : handlers) {
		if (h->test(event, component)) {
			return h->getCallback()(event, json, outJson);
		}
	}

	return nullptr;
}

JsonObject &SGraphQL::fillRootObject(const char *request_id, JsonObject &outJson) {
	// JsonObject & root = outJson.createNestedObject(SGraphQL::RESP_ROOT_NAME);
	String resp_id = String(millis());
	outJson[RESP_ID] = resp_id;
	outJson[RESP_REQ_ID] = request_id;
	return outJson;
}

bool SGraphQL::validRequest(const JsonObject &json) {
	const char *type;
	if (!json.success()) {
		logger.error("Not valid json, JsonObject not scuccess\n");
		return false;
	}
	if (!json.containsKey("id")) {
		logger.error("Not valid json, JsonObject not contain key \"id\"\n");
		return false;
	}
	if (!json.containsKey("type")) {
		logger.error("Not valid json, JsonObject not contain key \"type\"\n");
		return false;
	}
	if (!json.containsKey("component")) {
		logger.error("Not valid json, JsonObject not contain key \"comonent\"\n");
		return false;
	}
	if (!json.containsKey("cmd")) {
		logger.error("Not valid json, JsonObject not contain key \"cmd\"\n");
		return false;
	}
	type = json["type"];
	if (!utils::streq(type, QUERY) && !utils::streq(type, MUTATION) && !utils::streq(type, ACTION)) {
		logger.error("Not valid json, API type filed\n");
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////
/* ApiHandler =================================================== */
////////////////////////////////////////////////////////////////////

ApiHandler::ApiHandler(const char *component, const char *event, ApiHandlerFunction callback) : callback_fn{callback} {
	logger.debug("ApiHandle ctr -> component : %s\n", component);
	_type = utils::copynewstr(event);
	_component = utils::copynewstr(component);
}

ApiHandler::~ApiHandler() {
	logger.debug("Destroy ApiHandler\n");
	if (_type) {
		logger.debug("Delete [] event: %s\n", _type);
		delete[] _type;
	}
	if (_component) {
		logger.debug("Delete [] component: %s\n", _component);
		delete[] _component;
	}
}

char *ApiHandler::getEventName() const { return _type; }

char *ApiHandler::getComponentName() const { return _component; }

ApiHandlerFunction ApiHandler::getCallback() const { return callback_fn; }

bool ApiHandler::test(const char *event, const char *component) const { return utils::streq(_component, component) && (utils::streq(_type, SGraphQL::ALL) || utils::streq(_type, event)); }

///////////////////////////////////////////////////////////////////
/* ApiResult =================================================== */
///////////////////////////////////////////////////////////////////

ApiResult::ApiResult() : _actions{} {}

ApiResult::~ApiResult() {}

uint8_t ApiResult::then(AsyncWebServerRequest *request) {
	if (_actions.empty()) {
		return 0;
	}
	uint8_t err = _actions.front()(request);
	_actions.pop();
	return err;
}
