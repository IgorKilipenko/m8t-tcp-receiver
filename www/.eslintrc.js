module.exports = {
    parser: 'babel-eslint',
    parserOptions: {
        ecmaVersion: 2017,
        ecmaFeatures: {
            experimentalObjectRestSpread: true,
            experimentalDecorators: true,
            jsx: true
        },
        sourceType: 'module'
    },
    env: {
        browser: true,
        node: true,
        es6: true
    },
    ecmaFeatures: {
        jsx: true
    },
    rules: {
        strict: [2, 'never'],
        'prettier/prettier': 'warn',
    },
    plugins: [
        'flowtype',
        'babel',
        'prettier',
        'html'
    ],
    extends: [
        "plugin:flowtype/recommended"
    ],
};