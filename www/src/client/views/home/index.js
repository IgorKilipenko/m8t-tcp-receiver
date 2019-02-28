import React from 'react';
import PropTypes from 'prop-types';
import classNames from 'classnames';
import { Helmet } from 'react-helmet';
import { withStyles } from '@material-ui/core/styles';

const styles = () =>{
    root: {
        display: 'flex'
    }
}

class HomeView extends React.Component {
    render(){
        const {classes} = this.props;
        return(
            <div className={classes.root}>
                <h1>
                    Home page
                </h1>
            </div>
        )
    }
}

export default withStyles(styles, { withTheme: true })(HomeView);