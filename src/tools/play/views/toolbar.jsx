import Menu from 'swoll/menu';
import View from 'swoll/view';

import {css, cx} from '@emotion/css';

const buttonClass = color => css({
  position: 'relative',
  marginLeft: '0.125rem',
  padding: '0.5rem',
  minWidth: '2rem',
  backgroundColor: `rgb(var(--color-${color}-normal))`,
  border: `1px solid rgb(var(--color-bg-normal))`,
  lineHeight: '1rem',
  '&:hover': {
    backgroundColor: `rgb(var(--color-${color}-light))`,
    borderColor: `rgb(var(--color-${color}-normal))`,
  },
  '&:active': {
    backgroundColor: `rgb(var(--color-${color}-dark))`,
    left: '1px',
    top: '1px',
  },
});

export default class Toolbar extends View {
  themeMenu;
  settingsMenu;

  constructor() {
    super();

    const setTheme = this.setTheme.bind(this);
    const onClick = function() { setTheme(['default', 'light', 'dark'][this.props.key]) };

    this.themeMenu = new Menu(
      new Menu.Title('Theme'),
      new Menu.Item({ key: 0, onClick }, 'Follow System'),
      new Menu.Separator,
      new Menu.Item({ key: 1, onClick }, 'Light'),
      new Menu.Item({ key: 2, onClick }, 'Dark'),
    );

    this.settingsMenu = new Menu(
      new Menu.Title('Tools'),
      new Menu.Item('Builder'),
      new Menu.Item('Shizz'),
      new Menu.Separator,
      new Menu.Title('Settings'),
      new Menu.Item('Configurações'),
    );
  }

  onCreate() {
    return (
      <div class={css({
        position: 'relative',
        left: 0,
        right: 0,
        backgroundColor: 'rgb(var(--color-bg-light))',
        fontSize: '1.25rem',
        lineHeight: '1.75rem',
        padding: '0.5rem 0.375rem',
      })}>
        <div class={css({float: 'right', paddingRight: '0.125rem'})}>
          <button
            class={cx('fas', 'fa-palette', buttonClass('primary'))}
            onClick={e => this.themeMenu.toggle(e.target)}>
          </button>
          <button
            class={cx('fas', 'fa-cog', buttonClass('primary'))}
            onClick={e => this.settingsMenu.toggle(e.target)}>
          </button>
        </div>
        <button class={buttonClass('bg')}>B</button>
        <button class={buttonClass('primary')}>1</button>
        <button class={buttonClass('secondary')}>2</button>
        <button class={buttonClass('tertiary')}>3</button>
        <button class={buttonClass('complement')}>C</button>
      </div>
    );
  }

  setTheme(theme) {
    let currentTheme = null;
    for (let c of document.body.classList.values()) {
      if (c.startsWith('theme-')) {
        currentTheme = c;
      }
    }
    if (theme === 'default') {
      if (currentTheme) {
        document.body.classList.remove(currentTheme);
      }
      return;
    }

    if (currentTheme) {
      document.body.classList.replace(currentTheme, `theme-${theme}`);
    } else {
      document.body.classList.add(`theme-${theme}`);
    }
  }
}