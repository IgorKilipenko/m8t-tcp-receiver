import React from 'react';
import ReactDOM from 'react-dom';
import CssBaseline from '@material-ui/core/CssBaseline';
import App from './views/app';
import { MuiThemeProvider, createMuiTheme } from '@material-ui/core/styles';
import { Provider } from 'mobx-react';
import {Router, Switch } from 'react-router-dom';
import { RouterStore, syncHistoryWithStore } from 'mobx-react-router';
import createBrowserHistory from 'history/createBrowserHistory';
import ApiStore from './stores/api-store';
import ScrollRouterStore from './stores/scroll-router-store';
import routes, {ScrollRouter} from './routes';

const theme = createMuiTheme({
    typography: {
        useNextVariants: true
    }
});

const browserHistory = createBrowserHistory();
const scrollRoutingStore = new ScrollRouterStore(routes);
const apiStore = new ApiStore();

const stores = {
    routing: scrollRoutingStore,
    apiStore: apiStore
};
const history = syncHistoryWithStore(browserHistory, scrollRoutingStore);
ReactDOM.render(
    <Provider {...stores}>
        <Router history={history}>
            <MuiThemeProvider theme={theme}>
                <CssBaseline />
                <App>
                    <Switch>
                        <ScrollRouter />
                    </Switch>
                </App>
            </MuiThemeProvider>
        </Router>
    </Provider>,
    document.getElementById('app')
);
