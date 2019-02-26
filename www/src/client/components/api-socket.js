import axios from 'axios';
import { async } from 'q';

export default class ApiSocket {
    constructor() {
        this.instance = axios.create({
            baseURL: 'http://192.168.1.62',
            timeout: 6000
        });
    }
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
