import React from 'react';
import ReactDOM from 'react-dom';
import CssBaseline from '@material-ui/core/CssBaseline';
import App from './views/app';
import { MuiThemeProvider, createMuiTheme } from '@material-ui/core/styles';
import { Provider } from 'mobx-react';
import createBrowserHistory from 'history/createBrowserHistory';
import ApiStore from './stores/api-store';

const theme = createMuiTheme({
    typography: {
        useNextVariants: true
    }
});

const browserHistory = createBrowserHistory();

const apiStore = new ApiStore();

const stores = {
    apiStore: apiStore
};

ReactDOM.render(
    <Provider {...stores}>
        <MuiThemeProvider theme={theme}>
            <CssBaseline />
            <App />
        </MuiThemeProvider>
    </Provider>,
    document.getElementById('app')
);
