import { observable, computed, action } from 'mobx';
import { Server } from 'https';

export default class ApiStore{
    @observable state = {server:null}

    @action
    setState(state) {
        if (typeof state == 'function') {
            state = state(this.state);
        }
        this.state = {
            ...this.state,
            ...state
        };
    }

    @computed
    get getState() {
        return this.state;
    }
}