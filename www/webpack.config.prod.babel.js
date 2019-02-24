import merge from 'webpack-merge';
import common from './webpack.config.common.babel';

const prodConfig = merge(common, {
    mode: 'production'
});

module.exports = prodConfig;
