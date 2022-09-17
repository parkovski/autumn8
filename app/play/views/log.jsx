import View from './view';
import {css, cx} from '@emotion/css';

const LogClass = css({
  position: 'fixed',
  bottom: 0,
  height: '200px',
  width: '100%',
  border: '1px solid rgb(var(--color-bg-dark))',
  backgroundColor: 'rgb(var(--color-bg-light))',
  color: 'rgb(var(--color-fg-normal))',
  fontFamily: 'monospace',
  overflowY: 'scroll',

  '& > p': {
    margin: 0,
  },
});

const LogOptionsClass = css({
  float: 'right',
  clear: 'left',
});

export default class LogView extends View {
  onCreate() {
    return (
      <div class={LogClass}>
        <div class={LogOptionsClass}>
          <button class={cx('fas', 'fa-ban')}></button>
          <button class={cx('fas', 'fa-ellipsis')}></button>
        </div>
        <p>This is a log entry.</p>
        <p>This is another one.</p>
      </div>
    );
  }
}