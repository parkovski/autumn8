import './style.css';

document.addEventListener('DOMContentLoaded', () => {
  const p = document.getElementById('para')!;
  setInterval(() => {
    if (p.classList.contains('text-white')) {
      p.classList.replace('text-white', 'text-blue-500');
    } else {
      p.classList.replace('text-blue-500', 'text-white');
    }
  }, 1000);
});
