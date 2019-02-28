import { observable, computed, action } from 'mobx';
import { RouterStore } from 'mobx-react-router';


export default class ScrollRouterStore extends RouterStore {
    @observable route;
    @observable branch;
    @observable routes;
    constructor(routes){
        super();
        this.updateRoutes(routes);
    }
    

    @action
    update(route, branch,) {
        if (route !== this.route) { 
            this.route = route;
        }
        if (branch !== this.branch){
            if (branch && branch.routes) {
                branch.routes = branch.routes.filter(r => !r.noView);
            }
            this.branch = branch;
        }
    }
    @action
    updateRoutes(routes) {
        if (this.routes !== routes) {
            this.routes = routes;
        }
    }

    @computed
    get getRoute() {
        return this.route;
    }
    @computed 
    get getBranch(){
        return this.branch;
    }
    @computed
    get getRoutes(){
        return this.routes;
    }
    @action
    goNext() {
        if (!this.isScroll) return;
        const current = this.getRoute;
        const branch = this.getBranch;
        const currentIndex = branch.routes.findIndex(r => r.path === current.path);
        const nextIndex = this.getNextIndex(currentIndex, branch.routes.length -1);
        const next = branch.routes[nextIndex];
        super.push(next.path);
        this.update(next, branch)
    }
    @action
    goPrev() {
        if (!this.isScroll) return;
        const current = this.getRoute;
        const branch = this.getBranch;
        const currentIndex = branch.routes.findIndex(r => r.path === current.path);
        const prevIndex = this.getPreviousIndex(currentIndex);
        const prev = branch.routes[prevIndex];
        super.push(prev.path);
        this.update(prev, branch)
    }

    getNextIndex(current, last){
        if (current >= last) {
            return current
        }
        return ++current;
    }
    getPreviousIndex(current) {
        if (current === 0){
            return 0;
        }
        return --current;
    }

    @computed
    get isScroll(){
        return this.getRoute && this.getBranch
    }
}
