import axios from 'axios';

export default class ApiSocket {
    constructor() {
        this.instance = axios.create({
            baseURL: `http://${REMOTE_API_URL}`, //'http://192.168.1.62',
            timeout: 6000,
            method: 'post',
            //maxContentLength: 40000
        });
        this.headers = {
            json: {
                'Content-Type': 'application/json'
            }
        };
        this.types = {
            query: 'query',
            mutation: 'mutation',
            action: 'action'
        };
        this.components = {
            receiver: 'receiver',
            wifi: 'wifi'
        };

        console.log({baseUrl:`http://${REMOTE_API_URL}`})
    }

    getWifiList = () => {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await this.instance({
                    method: 'post',
                    //url: '/api',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    data: {
                        type: this.types.query,
                        component: this.components.wifi,
                        cmd: 'scan'
                    }
                });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    };

    connectWiFiSTA = (ssid, password) => {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await this.instance({
                    method: 'post',
                    url: '/service',
                    headers: this.headers.json,
                    data: {
                        type: this.types.action,
                        component: this.components.wifi,
                        cmd: 'connect',
                        ssid,
                        password
                    }
                });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    }
}
