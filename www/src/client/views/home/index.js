import React from 'react';
import PropTypes from 'prop-types';
import classNames from 'classnames';
import { Helmet } from 'react-helmet';
import { withStyles } from '@material-ui/core/styles';
import ReceiverView from '../receiver';

const styles = (theme) =>{
    root: {
        display: 'flex'
    }
}

class HomeView extends React.Component {
    render(){
        const {classes} = this.props;
        return(
            <div className={classes.root}>
                <ReceiverView/>
            </div>
        )
    }
}

export default withStyles(styles, { withTheme: true })(HomeView);