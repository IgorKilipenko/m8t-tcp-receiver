import axios from 'axios';

export default class ApiSocket {
    constructor() {
        this.instance = axios.create({
            baseURL: DEVELOPMENT ? `http://${REMOTE_API_URL}` : API_URL,
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

        console.log({baseUrl:this.instance.baseURL})
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
                        cmd: 'scan',
                        id: Date.now()
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
                    //baseURL: 'http://192.168.1.62',
                    data: {
                        type: this.types.action,
                        component: this.components.wifi,
                        cmd: 'connect',
                        ssid,
                        password,
                        id: Date.now()
                    }
                });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    }
}
