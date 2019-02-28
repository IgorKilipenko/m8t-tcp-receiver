import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import TextField from '@material-ui/core/TextField';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import InputAdornment from '@material-ui/core/InputAdornment';
import IconButton from '@material-ui/core/IconButton';
import classNames from 'classnames';
import Visibility from '@material-ui/icons/Visibility';
import VisibilityOff from '@material-ui/icons/VisibilityOff';

const styles = theme => ({
    margin: {
        margin: theme.spacing.unit
    },
    textField: {
        marginLeft: theme.spacing.unit,
        marginRight: theme.spacing.unit,
        width: 'auto'
    }
});

class LoginDialog extends React.Component {
    state = {
        showPassword: false,
        login: ''
    };

    handleClose = () => {
        this.setState({ open: false });
    };

    handleChange = prop => event => {
        this.setState({ [prop]: event.target.value });
    };

    handleClickShowPassword = () => {
        this.setState(state => ({ showPassword: !state.showPassword }));
    };

    render() {
        const { classes, open, ssid, onCloseDialog } = this.props;
        return (
            <div>
                <Dialog
                    open={open}
                    onClose={this.handleClose}
                    aria-labelledby="form-dialog-title"
                >
                    <DialogTitle id="form-dialog-title">
                        {`Enter yore login ${ssid ? 'to ' + ssid : ''}`}
                    </DialogTitle>
                    <DialogContent>
                        <TextField
                            id="outlined-adornment-login"
                            className={classNames(
                                classes.margin,
                                classes.textField
                            )}
                            //variant="outlined"
                            label="Login"
                            value={this.state.weight}
                            onChange={this.handleChange('login')}
                        />
                        <TextField
                            id="outlined-adornment-password"
                            className={classNames(
                                classes.margin,
                                classes.textField
                            )}
                            //variant="outlined"
                            type={this.state.showPassword ? 'text' : 'password'}
                            label="Password"
                            autoComplete="current-password"
                            value={this.state.password}
                            onChange={this.handleChange('password')}
                            InputProps={{
                                endAdornment: (
                                    <InputAdornment position="end">
                                        <IconButton
                                            aria-label="Toggle password visibility"
                                            onClick={
                                                this.handleClickShowPassword
                                            }
                                        >
                                            {this.state.showPassword ? (
                                                <VisibilityOff />
                                            ) : (
                                                <Visibility />
                                            )}
                                        </IconButton>
                                    </InputAdornment>
                                )
                            }}
                        />
                    </DialogContent>
                    <DialogActions>
                        <Button
                            onClick={() => {
                                onCloseDialog &&
                                    onCloseDialog(
                                        this.state.login,
                                        this.state.password,
                                        false
                                    );
                            }}
                            color="primary"
                        >
                            Cancel
                        </Button>
                        <Button
                            onClick={() => {
                                onCloseDialog &&
                                    onCloseDialog(
                                        this.state.login,
                                        this.state.password,
                                        true
                                    );
                            }}
                            color="primary"
                        >
                            Save
                        </Button>
                    </DialogActions>
                </Dialog>
            </div>
        );
    }
}

LoginDialog.propTypes = {
    classes: PropTypes.object.isRequired,
    open: PropTypes.bool.isRequired,
    onCloseDialog: PropTypes.func.isRequired,
    ssid: PropTypes.string
};

export default withStyles(styles, { withTheme: true })(LoginDialog);
