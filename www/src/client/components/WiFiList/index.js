import React from 'react';
import PropTypes from 'prop-types';
import classNames from 'classnames';
import { withStyles } from '@material-ui/core/styles';
const api = new ApiSocket();

class WiFiList extends React.Component {
    state = {
        open: false,
        status: '',
        data: [
            {
                '1': 'rssi',
                '2': 'ssid',
                '3': 'bssid',
                '4': 'channel',
                '5': 'secure',
                '6': 'hidden'
            }
        ]
    };

    getWifiLiset = async () => {
        try {
            const resp = await axios({
                method: 'get',
                url: 'http://192.168.1.62/api/wifi/scan',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            this.setState({
                staus: resp.status,
                data: resp.data
            });
        } catch (err) {
            console.log({ err });
        }
    };
}