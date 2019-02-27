import axios from 'axios';

export default class ApiSocket {
    constructor() {
        this.instance = axios.create({
            baseURL: 'http://192.168.1.62',
            timeout: 6000,
            method: 'post',
            maxContentLength: 40000
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
    }

    getWifiList = () => {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await this.instance({
                    method: 'post',
                    url: '/api',
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

    getServerInfo = () => {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await this.instance({
                    url: '/consts',
                    method: 'get'
                });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    };

    getConstants = () => {
        return new Promise(async (reslove, reject) => {
            try {
                const resp = await this.instance({
                    url: '/consts',
                    method: 'get'
                });
                reslove(resp);
            } catch (err) {
                reject(err);
            }
        });
    };

    getGpsStatus = async () => {
        try {
            const resp = await this.instance({
                method: 'post',
                url: '/gnss',
                data: {}
            });
            this.setState({ gps });
        } catch (err) {
            console.log({ err });
        }
    };
}
