document.addEventListener('DOMContentLoaded', function() {
  let count = 0;
  const countSpan = document.getElementById('count');
  const button = document.getElementById('btn');
  button.addEventListener('click', function() {
    countSpan.innerText = '' + (++count);
  });
});
