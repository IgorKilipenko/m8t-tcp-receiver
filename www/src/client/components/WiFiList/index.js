import React, { Fragment } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemIcon from '@material-ui/core/ListItemIcon';
import ListItemText from '@material-ui/core/ListItemText';
import ListSubheader from '@material-ui/core/ListSubheader';
import Switch from '@material-ui/core/Switch';
import WifiIcon from '@material-ui/icons/Wifi';
import LoginDialog from '../LoginDialog';

const styles = theme => ({
    root: {
        width: '100%',
        maxWidth: 360,
        backgroundColor: theme.palette.background.paper
    }
});



class WiFiList extends React.Component {
    state = {
        openLoginDialog: false,
    };

    handleClick = (wifi) =>{
        this.setState({
            openLoginDialog: true,
            currentSsid:wifi.ssid
        });

    }

    handleCloseLoginDialog = ({login, password, save}) => {
        this.setState({openLoginDialog:false});
    }

    render() {
        const { classes, wifiListData } = this.props;
        wifiListData.sort((w1, w2) => w2.rssi - w1.rssi);
        return (
            <Fragment>
            <LoginDialog open={this.state.openLoginDialog} onCloseDialog={this.handleCloseLoginDialog} ssid={this.state.currentSsid}/>
            <List
                subheader={<ListSubheader>Settings</ListSubheader>}
                className={classes.root}
            >
                {wifiListData.map((wifi, i) => {
                    return (
                        <ListItem key={i} button onClick={() => this.handleClick(wifi)}>
                            <ListItemIcon>
                                <WifiIcon />
                            </ListItemIcon>
                            <ListItemText primary={`${wifi.ssid || 'ssid'}\t|\t${wifi.rssi || rssi}`} />
                        </ListItem>
                    );
                })}
            </List>
            </Fragment>
        );
    }
}

WiFiList.propTypes = {
    classes: PropTypes.object.isRequired,
    wifiListData: PropTypes.array.isRequired
};

export default withStyles(styles, { withTheme: true })(WiFiList);