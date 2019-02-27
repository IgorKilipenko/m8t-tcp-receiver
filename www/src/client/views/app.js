import React from 'react';
import PropTypes from 'prop-types';
import { Helmet } from 'react-helmet';
import { withStyles } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import MiniDrawer from '../components/root';
//import axios from 'axios';
import ApiSocket from '../components/api-socket';
import { inject, observer } from 'mobx-react';

const api = new ApiSocket();

const styles = theme => ({});

@inject('apiStore')
@observer
class App extends React.Component {
    constructor() {
        super();
        this.state = {
            gps: {},
            server: {}
        };
    }
    sendGnssCmd = () => {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await api.instance({
                    method: 'post',
                    url: '/api/gnss/cmd',
                    params: this.state.gps.enabled ? { cmd: '0' } : { cmd: '1' }
                });
                console.log({ resp });
                const gps = {
                    respStatus: resp.status,
                    data: resp.data,
                    enabled: resp.data && resp.data.enabled
                };
                console.log({ gps });
                this.setState({ gps });
                reslove(gps);
            } catch (err) {
                reject(err);
            }
        });
    };

    testRestApi() {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await api.instance({
                    method: 'post',
                    url: '/rest/endpoint',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    data: JSON.stringify({
                        rest_test: 'TEST SUCCESS'
                    })
                });
                console.log({ resp });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    }

    testApi() {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await api.instance({
                    method: 'post',
                    url: '/api',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    data: JSON.stringify({
                        type: 'query',
                        component: 'receiver'
                    })
                });
                console.log({ resp });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    }

    componentDidMount = async () => {
        //try {
        //    this.getGpsState();
        //} catch (err) {
        //    console.log({ err }, this);
        //}

        try {
            //this.testRestApi();
            //this.testApi()
            const res = await api.getWifiList();
            console.log({res});
        } catch (err) {
            console.log({err});
        }


    };

    render() {
        const { gps } = this.state;
        return (
            <div>
                <Helmet>
                    {/*<meta charSet="ANSI" />*/}
                    {/*<meta charSet="Windows-1252" />*/}
                    <meta charSet="utf-8" />
                    <title>ESP GPS</title>
                    {/*<link rel="icon" href={favicon}/>
                    <meta name="msapplication-TileImage" content={favicon}/>*/}
                    <meta name="theme-color" content="#9CC2CE" />
                    <meta
                        name="viewport"
                        content="minimum-scale=1, initial-scale=1, width=device-width, shrink-to-fit=no"
                    />
                    <link
                        rel="stylesheet"
                        href="https://fonts.googleapis.com/css?family=Roboto:300,400,500"
                    />
                    <link
                        href="https://fonts.googleapis.com/css?family=Montserrat+Alternates:300,300i,400,400i,500,500i,600,600i&amp;subset=cyrillic"
                        rel="stylesheet"
                    />
                    <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons"></link>
                </Helmet>
                <MiniDrawer>
                    <Button onClick={async () => this.sendGnssCmd()}>
                        {gps.enabled ? 'Stop GPS' : 'Start GPS'}
                    </Button>
                    <Button>Тест utf-8</Button>
                </MiniDrawer>
            </div>
        );
    }
}

export default withStyles(styles, { withTheme: true })(App);
