//const colors = require('tailwindcss/colors');

const themes = {
  light: {
    primary: '#ee2500',
    secondary: '#ee8300',
    tertiary: '#086297',
    complement: '#00ae3b',
  },
  dark: {
    primary: '#06673d',
    secondary: '#418607',
    tertiary: '#0b3c61',
    complement: '#973108',
  },
};

module.exports = {
  content: ['./src/tools/**/*.{html,css,js,jsx,ts,tsx}'],
  theme: {
    extend: {
      colors: themes.light,
    },
  },
  plugins: [],
};
