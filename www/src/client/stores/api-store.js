import { observable, computed, action } from 'mobx';
import { Server } from 'https';

export default class ApiStore{
    @observable _receiverState = {
        enabled:false,
        timeStart:0,
        serverStart:0
    }

    @action
    setReceiverState(state) {
        if (typeof state == 'function') {
            state = state(this._receiverState);
        }
        this._receiverState = {
            ...this._receiverState,
            ...state
        };
    }

    @computed
    get receiverState() {
        return this._receiverState;
    }

    @computed
    get timeReceive(){
        const {enabled, timeStart, serverStart} = this._receiverState;
        console.log({enabled, timeStart, serverStart})
        if (!enabled || !timeStart){
            return 0;
        }
        return Date.now() - timeStart - serverStart;
    }
}